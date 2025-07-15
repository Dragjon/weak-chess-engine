// error 0.0764196

#include <stdint.h>

#include "chess.hpp"
#include "packing.hpp"
#include "eval.hpp"
#include "bitboard.hpp"

using namespace chess;
using namespace std;

// Entire evaluation function is tuned with non other than Gedas' Texel tuner <3
// https://github.com/GediminasMasaitis/texel-tuner

/////////////////////////////////
// HCE Evaluation description ///
/////////////////////////////////

// 1. 
// PST
// Piece Square Tables (which includes the material value of each piece)
// Each piece type has its own piece square table, whiched it used
// to evaluate its positioning on the board

// 2. 
// Mobility Eval
// Each piece type can attack at most 28 squares in any given turn. In
// general, the more squares a piece attacks the better. But for example
// pieces like queens may get negative mobility in the opening to prevent
// early queen moves. The 4th index (index 5) of the mobilities array is 
// dedicated to king virtual mobility. That is putting a queen on a king sq
// and getting the mobility of the queen from there. This helps to position
// the king away from open rays

// 3.
// Bishop pair evaluation
// Bishop pair eval is a pretty good eval heuristic for HCE engines and is
// pretty intuitive to implement.

// 4.
// Passed pawn bonus
// Passed pawns are given a bonus indexed by which square they are on. Passed
// pawns that are more advanced generally have higher scores.

// 5. / 6.
// King zone attacks 
// Bonus for pieces (not pawns or kings) which attack squares near the opponent
// king. This includes the square directly surrounding the opponent king and also
// 1 square away from the king

// 7. 
// Doubled pawn penalty
// Doubled pawns are given a penalty depending on which file they are on. This is
// a pretty strong eval feature and is also easy to understand. Doubled pawns
// (NOT stacked pawns) are pawns that are on the same file. It is usually bad
// to have doubled pawns

// 8.
// Pawn storm
// Pawns on the other half of the board without the king are given a different
// bonus-table. This could encourage those pawns to move more freely and participate
// in a "pawn storm" to attack the king

// 9.
// Isolated pawns
// Isolated pawns are given a penalty depending on which square they are on. Being
// isolated is defined as pawns whose adjacent files have no other friendly pawns.

// 10.
// Threats
// Threats are attacks that are made by a friendly piece to an enemy piece. This is
// indexed by [our piece][their piece]. This could make our evaluation more accurate

// 11.0
// Tempo
// Pretty controversial whether it is an eval or search feature. Also tuned with texel 
// tuner (some witchcraft here)

const int32_t PSQT[6][64] = {
    {
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
        S(43, 189), S(76, 203), S(67, 185), S(76, 150), S(77, 122), S(76, 134), S(51, 155), S(32, 165),
        S(82, 164), S(93, 184), S(91, 171), S(95, 153), S(113, 112), S(148, 118), S(135, 158), S(104, 139),
        S(81, 133), S(98, 141), S(94, 128), S(66, 123), S(92, 106), S(92, 112), S(88, 123), S(79, 104),
        S(78, 110), S(83, 124), S(90, 109), S(77, 120), S(81, 107), S(84, 103), S(74, 112), S(72, 92),
        S(91, 102), S(102, 111), S(91, 107), S(54, 120), S(82, 113), S(81, 107), S(96, 104), S(82, 88),
        S(83, 99), S(113, 102), S(101, 99), S(73, 114), S(74, 123), S(89, 105), S(104, 98), S(74, 81),
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
    },
    {
        S(17, 81), S(34, 145), S(67, 127), S(94, 118), S(130, 116), S(55, 100), S(55, 138), S(67, 57),
        S(109, 151), S(109, 131), S(85, 127), S(103, 127), S(97, 117), S(120, 113), S(107, 125), S(131, 131),
        S(112, 122), S(105, 126), S(158, 176), S(165, 178), S(177, 172), S(200, 155), S(111, 122), S(124, 113),
        S(121, 133), S(105, 136), S(158, 182), S(173, 188), S(154, 188), S(176, 185), S(106, 139), S(147, 124),
        S(106, 132), S(95, 130), S(141, 182), S(141, 185), S(149, 188), S(145, 172), S(112, 127), S(115, 123),
        S(89, 117), S(81, 123), S(123, 164), S(133, 177), S(144, 174), S(128, 157), S(98, 115), S(105, 117),
        S(107, 147), S(89, 124), S(72, 122), S(87, 121), S(86, 118), S(84, 117), S(103, 112), S(128, 153),
        S(81, 141), S(114, 137), S(82, 122), S(95, 121), S(98, 121), S(100, 111), S(114, 145), S(112, 129)
    },
    {
        S(116, 176), S(101, 180), S(102, 171), S(66, 180), S(82, 175), S(90, 169), S(122, 167), S(88, 169),
        S(115, 169), S(122, 170), S(123, 168), S(114, 167), S(112, 163), S(125, 166), S(102, 177), S(106, 169),
        S(129, 180), S(141, 171), S(135, 170), S(140, 163), S(131, 168), S(157, 172), S(145, 171), S(133, 179),
        S(120, 176), S(123, 174), S(132, 171), S(150, 180), S(141, 174), S(135, 175), S(121, 171), S(123, 172),
        S(125, 172), S(115, 175), S(125, 171), S(141, 174), S(141, 170), S(131, 166), S(125, 169), S(131, 162),
        S(127, 170), S(138, 168), S(136, 168), S(133, 168), S(136, 171), S(139, 164), S(142, 160), S(147, 160),
        S(146, 171), S(140, 156), S(144, 152), S(131, 163), S(137, 161), S(144, 157), S(155, 161), S(150, 154),
        S(137, 162), S(148, 171), S(136, 163), S(129, 168), S(134, 166), S(128, 174), S(139, 160), S(159, 145)
    },
    {
        S(175, 314), S(165, 320), S(169, 327), S(163, 325), S(170, 319), S(195, 314), S(178, 318), S(197, 310),
        S(175, 311), S(170, 320), S(181, 324), S(196, 315), S(179, 318), S(205, 308), S(202, 306), S(223, 296),
        S(167, 308), S(188, 304), S(179, 307), S(179, 302), S(205, 297), S(208, 291), S(231, 290), S(208, 288),
        S(162, 312), S(168, 307), S(171, 312), S(178, 308), S(174, 299), S(182, 294), S(177, 297), S(180, 296),
        S(146, 311), S(146, 310), S(154, 310), S(165, 308), S(162, 307), S(152, 302), S(162, 295), S(156, 296),
        S(140, 310), S(145, 304), S(153, 304), S(156, 306), S(159, 302), S(156, 293), S(176, 277), S(161, 285),
        S(142, 303), S(151, 301), S(165, 302), S(167, 300), S(168, 295), S(164, 289), S(176, 281), S(153, 290),
        S(152, 304), S(158, 303), S(166, 306), S(171, 301), S(171, 297), S(155, 300), S(168, 291), S(155, 290)
    },
    {
        S(338, 566), S(332, 573), S(348, 587), S(373, 572), S(359, 578), S(371, 579), S(412, 529), S(370, 563),
        S(360, 564), S(345, 574), S(342, 601), S(332, 616), S(321, 636), S(367, 588), S(374, 579), S(397, 585),
        S(363, 566), S(359, 568), S(358, 589), S(357, 593), S(357, 602), S(390, 584), S(394, 565), S(380, 577),
        S(345, 584), S(352, 581), S(348, 587), S(347, 600), S(346, 602), S(352, 594), S(358, 598), S(356, 591),
        S(350, 571), S(342, 586), S(344, 581), S(349, 595), S(346, 590), S(346, 583), S(350, 577), S(356, 582),
        S(348, 556), S(352, 563), S(348, 577), S(341, 573), S(346, 578), S(346, 573), S(360, 553), S(357, 561),
        S(353, 553), S(352, 553), S(358, 550), S(353, 562), S(352, 563), S(355, 535), S(361, 514), S(378, 501),
        S(345, 561), S(349, 551), S(351, 555), S(350, 570), S(353, 547), S(332, 547), S(345, 536), S(357, 531)
    },
    {
        S(71, -110), S(84, -64), S(112, -52), S(-7, -6), S(-35, 7), S(-38, 10), S(35, -10), S(156, -108),
        S(-51, -23), S(42, -2), S(20, 12), S(110, 3), S(16, 44), S(9, 52), S(19, 36), S(-31, 14),
        S(-81, -13), S(72, -3), S(36, 21), S(18, 36), S(17, 64), S(64, 47), S(12, 38), S(-42, 15),
        S(-46, -23), S(-3, 0), S(-9, 24), S(-41, 47), S(-68, 74), S(-51, 60), S(-64, 38), S(-133, 27),
        S(-59, -27), S(-11, -6), S(-22, 19), S(-45, 38), S(-68, 71), S(-40, 49), S(-58, 33), S(-133, 23),
        S(-26, -30), S(21, -14), S(-4, 6), S(-12, 17), S(-27, 50), S(-21, 39), S(-2, 19), S(-51, 10),
        S(32, -43), S(32, -20), S(23, -9), S(2, -1), S(-19, 35), S(-5, 25), S(24, 7), S(5, -8),
        S(19, -79), S(46, -59), S(25, -39), S(-33, -21), S(-8, -5), S(-31, 6), S(16, -17), S(2, -44)
    },
};


const int32_t mobilities[5][28] = {

    {
        S(0, 0), S(0, 0), S(124, 204), S(142, 199), S(168, 235), S(0, 0), S(203, 248), S(0, 0), S(175, 214), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(118, 150), S(137, 144), S(142, 176), S(156, 185), S(160, 194), S(171, 209), S(176, 215), S(182, 223), S(184, 227), S(186, 233), S(186, 230), S(187, 228), S(204, 224), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(0, 0), S(198, 311), S(203, 329), S(208, 337), S(211, 343), S(212, 348), S(213, 354), S(217, 357), S(221, 361), S(225, 366), S(228, 371), S(231, 377), S(238, 381), S(239, 385), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(0, 0), S(0, 0), S(457, 340), S(436, 444), S(462, 488), S(461, 517), S(459, 572), S(462, 585), S(460, 606), S(463, 612), S(465, 623), S(466, 634), S(468, 637), S(471, 643), S(471, 651), S(470, 657), S(472, 662), S(471, 668), S(472, 672), S(473, 679), S(476, 677), S(477, 679), S(487, 671), S(485, 673), S(501, 665), S(535, 644), S(578, 619),
    },
    {
        S(0, 0), S(0, 0), S(0, 0), S(45, -10), S(51, -21), S(32, -3), S(26, -1), S(24, -7), S(21, -6), S(16, -6), S(15, -5), S(8, 0), S(6, -2), S(-1, 0), S(-10, 3), S(-21, 5), S(-34, 6), S(-50, 7), S(-62, 8), S(-78, 8), S(-77, 4), S(-82, 1), S(-90, 0), S(-98, -3), S(-111, -4), S(-99, -14), S(-118, -11), S(-96, -24),
    },
};


const int32_t bishop_pair = S(18, 58);


const int32_t passed_pawns[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(43, 189), S(76, 203), S(67, 185), S(76, 150), S(77, 122), S(76, 134), S(51, 155), S(32, 165),
    S(20, 144), S(32, 151), S(20, 115), S(5, 78), S(9, 89), S(14, 103), S(-17, 116), S(-39, 144),
    S(14, 79), S(19, 76), S(19, 54), S(15, 49), S(5, 47), S(11, 54), S(0, 71), S(-10, 81),
    S(3, 43), S(1, 37), S(-11, 29), S(-4, 23), S(-11, 26), S(-5, 29), S(4, 39), S(1, 39),
    S(0, 7), S(-6, 14), S(-16, 12), S(-17, 6), S(-13, 6), S(-4, 5), S(11, 19), S(15, 7),
    S(-6, 7), S(-2, 11), S(-16, 11), S(-13, 5), S(0, -7), S(4, -3), S(22, 1), S(8, 7),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};

const int32_t inner_king_zone_attacks[4] = {
    S(10, -7), S(18, -4), S(21, -8), S(13, 4),
};


const int32_t outer_king_zone_attacks[4] = {
    S(0, 1), S(0, 0), S(3, -4), S(2, 0),
};


const int32_t doubled_pawn_penalty[8] = {
    S(1, -38), S(1, -25), S(-3, -24), S(-2, -9), S(0, -20), S(-4, -22), S(0, -23), S(-5, -33),
};


const int32_t pawn_storm[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(21, -63), S(-35, -57), S(-3, -95), S(-19, -52), S(-57, 54), S(-43, -10), S(-7, -7), S(-40, 3),
    S(-22, -33), S(-19, -42), S(11, -53), S(9, -31), S(-10, 18), S(-28, 0), S(-47, -6), S(-54, 7),
    S(-34, -10), S(-29, -14), S(-21, -14), S(7, -19), S(-7, 13), S(-12, 2), S(-35, 11), S(-53, 18),
    S(-38, -4), S(-24, -6), S(-20, -3), S(4, -13), S(-3, 6), S(-15, 6), S(-30, 8), S(-51, 17),
    S(-52, 5), S(-44, 3), S(-24, -1), S(18, -9), S(-9, 1), S(-18, 1), S(-58, 18), S(-67, 25),
    S(-43, 10), S(-56, 10), S(-45, 9), S(-12, -1), S(-26, 4), S(-36, 6), S(-72, 27), S(-62, 39),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};

const int32_t isolated_pawns[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(44, -26), S(109, -108), S(48, -4), S(54, -5), S(19, 7), S(1, 31), S(-31, 25), S(0, 0),
    S(13, -18), S(13, -49), S(2, -24), S(4, -13), S(2, -17), S(-2, -9), S(21, -26), S(9, -27),
    S(8, -9), S(-2, -28), S(-8, -16), S(-7, -21), S(-6, -23), S(6, -21), S(16, -26), S(6, -10),
    S(-2, 0), S(-7, -12), S(-22, -6), S(-23, -20), S(-16, -19), S(-16, -7), S(-16, -8), S(-11, 0),
    S(-12, -4), S(-14, -16), S(-25, -11), S(-14, -17), S(-31, -16), S(-20, -9), S(-28, -14), S(-22, -2),
    S(-8, -4), S(-15, -7), S(-3, -11), S(-28, -6), S(-30, -10), S(-9, -6), S(-4, -15), S(-12, 1),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};

const int32_t threats[6][6] = {

    {
        S(-7, -2), S(20, -22), S(24, -3), S(0, -6), S(7, -38), S(-79, 29),
    },
    {
        S(-10, 7), S(0, 0), S(27, 33), S(58, 15), S(43, -16), S(0, 0),
    },
    {
        S(-6, 8), S(17, 26), S(0, 0), S(45, 26), S(57, 89), S(0, 0),
    },
    {
        S(-6, 16), S(9, 19), S(17, 16), S(0, 0), S(75, 22), S(0, 0),
    },
    {
        S(-4, 10), S(0, 10), S(-1, 32), S(2, 5), S(0, 0), S(0, 0),
    },
    {
        S(36, 38), S(-40, 16), S(-14, 26), S(-17, 12), S(0, 0), S(0, 0),
    },
};


const int32_t tempo = S(23, 24);


// For a tapered evaluation
const int32_t game_phase_increment[6] = {0, 1, 1, 2, 4, 0};

// This is our HCE evaluation function. 
int32_t evaluate(const chess::Board& board) {

    int32_t eval_array[2] = {0,0};
    eval_array[board.sideToMove() == Color::WHITE ? 0 : 1] += tempo;
    int32_t phase = 0;

    // Get all piece bitboards for efficient looping
    chess::Bitboard wp = board.pieces(chess::PieceType::PAWN, chess::Color::WHITE);
    chess::Bitboard wn = board.pieces(chess::PieceType::KNIGHT, chess::Color::WHITE);
    chess::Bitboard wb = board.pieces(chess::PieceType::BISHOP, chess::Color::WHITE);
    chess::Bitboard wr = board.pieces(chess::PieceType::ROOK, chess::Color::WHITE);
    chess::Bitboard wq = board.pieces(chess::PieceType::QUEEN, chess::Color::WHITE);
    chess::Bitboard wk = board.pieces(chess::PieceType::KING, chess::Color::WHITE);
    chess::Bitboard bp = board.pieces(chess::PieceType::PAWN, chess::Color::BLACK);
    chess::Bitboard bn = board.pieces(chess::PieceType::KNIGHT, chess::Color::BLACK);
    chess::Bitboard bb = board.pieces(chess::PieceType::BISHOP, chess::Color::BLACK);
    chess::Bitboard br = board.pieces(chess::PieceType::ROOK, chess::Color::BLACK);
    chess::Bitboard bq = board.pieces(chess::PieceType::QUEEN, chess::Color::BLACK);
    chess::Bitboard bk = board.pieces(chess::PieceType::KING, chess::Color::BLACK);
    chess::Bitboard all[] = {wp,wn,wb,wr,wq,wk,bp,bn,bb,br,bq,bk};

    int32_t whiteKingSq = board.kingSq(chess::Color::WHITE).index();
    int32_t blackKingSq = board.kingSq(chess::Color::BLACK).index();

    uint64_t not_kingside_w_mask = NOT_KINGSIDE_HALF_MASK[whiteKingSq];
    uint64_t not_kingside_b_mask = NOT_KINGSIDE_HALF_MASK[blackKingSq];

    uint64_t white_king_2_sq_mask = OUTER_2_SQ_RING_MASK[whiteKingSq];
    uint64_t black_king_2_sq_mask = OUTER_2_SQ_RING_MASK[blackKingSq];
    uint64_t white_king_inner_sq_mask = chess::attacks::king(whiteKingSq).getBits();
    uint64_t black_king_inner_sq_mask = chess::attacks::king(blackKingSq).getBits();

    // Keep track of rooks on open files for updating after loop
    // int32_t num_w_rooks_on_op_file = 0;
    // int32_t num_b_rooks_on_op_file = 0;

    // A fast way of getting all the pieces 
    for (int32_t i = 0; i < 12; i++){        
        chess::Bitboard curr_bb = all[i];
        while (!curr_bb.empty()) {
            int16_t sq = curr_bb.pop();
            bool is_white = i < 6;
            int16_t j = is_white ? i : i-6;

            // Phase for tapered evaluation
            phase += game_phase_increment[j];

            // Piece square tables
            eval_array[is_white ? 0 : 1] += PSQT[j][is_white ? sq ^ 56 : sq];

            uint64_t attacks_bb = 0ull;

            // Mobilities for knight - queen, and king virtual mobility
            // King Zone
            if (j > 0){
                int32_t attacks = 0;
                switch (j)
                {
                    // knights
                    case 1:
                        attacks_bb = chess::attacks::knight(static_cast<chess::Square>(sq)).getBits();
                        attacks = count(attacks_bb);
                        break;
                    // bishops
                    case 2:
                        attacks_bb = chess::attacks::bishop(static_cast<chess::Square>(sq), board.occ()).getBits();
                        attacks = count(attacks_bb);
                        break;
                    // rooks
                    case 3:
                        // Rook on open file
                        /*
                        if ((is_white ? (WHITE_AHEAD_MASK[sq] & wp & bp) : (BLACK_AHEAD_MASK[sq] & wp & bp)) == 0ull){
                            if (is_white) num_w_rooks_on_op_file++;
                            else num_b_rooks_on_op_file++;
                        }
                        */

                        attacks_bb = chess::attacks::rook(static_cast<chess::Square>(sq), board.occ()).getBits();
                        attacks = count(attacks_bb);
                        break;
                    // queens
                    case 4:
                        attacks_bb = chess::attacks::queen(static_cast<chess::Square>(sq), board.occ()).getBits();
                        attacks = count(attacks_bb);
                        break;
                    // King Virtual Mobility
                    case 5:
                        attacks = chess::attacks::queen(static_cast<chess::Square>(sq), board.occ()).count();
                        break;

                    default:
                        break;
                }
                eval_array[is_white ? 0 : 1] += mobilities[j-1][attacks];
                
                // Non king non pawn pieces
                if (j < 5){
                    eval_array[is_white ? 0 : 1] += inner_king_zone_attacks[j-1]  * count((is_white ? black_king_inner_sq_mask : white_king_inner_sq_mask) & attacks_bb); 
                    eval_array[is_white ? 0 : 1] += outer_king_zone_attacks[j-1]  * count((is_white ? black_king_2_sq_mask : white_king_2_sq_mask) & attacks_bb); 
                }
            }

            // Pawns
            else {
                // Doubled pawns
                // Note that for pawns to be considered "doubled", they need not be directly in front
                // of another pawn
                uint64_t front_mask = is_white ? WHITE_AHEAD_MASK[sq] : BLACK_AHEAD_MASK[sq];
                uint64_t our_pawn_bb = is_white ? wp.getBits() : bp.getBits();
                if (front_mask & our_pawn_bb)
                    eval_array[is_white ? 0 : 1] += doubled_pawn_penalty[is_white ? 7 - sq % 8 : sq % 8];

                // Passed pawn
                if (is_white ? is_white_passed_pawn(sq, bp.getBits()): is_black_passed_pawn(sq, wp.getBits())){
                    eval_array[is_white ? 0 : 1] += passed_pawns[is_white ? sq ^ 56 : sq];
                }

                // Pawn storm
                if (is_white ? (not_kingside_w_mask & (1ull << sq)) : (not_kingside_b_mask & (1ull << sq))){
                    eval_array[is_white ? 0 : 1] += pawn_storm[is_white ? sq ^ 56 : sq];
                }

                // Isolated pawn
                if ((LEFT_RIGHT_COLUMN_MASK[sq] & (is_white ? wp.getBits() : bp.getBits())) == 0ull){
                    eval_array[is_white ? 0 : 1] += isolated_pawns[is_white ? sq ^ 56 : sq];
                }
            }

            // Actual king attacks because we let attacks bb for king be a queen attacks bb
            // for virtual mobility above
            if (j == 5) attacks_bb = chess::attacks::king(static_cast<chess::Square>(sq)).getBits();

            // Threats
            int32_t num_pawn_attacks = is_white ? count(attacks_bb & bp.getBits()) : count(attacks_bb & wp.getBits());
            int32_t num_knight_attacks = is_white ? count(attacks_bb & bn.getBits()) : count(attacks_bb & wn.getBits());
            int32_t num_bishop_attacks = is_white ? count(attacks_bb & bb.getBits()) : count(attacks_bb & wb.getBits());
            int32_t num_rook_attacks = is_white ? count(attacks_bb & br.getBits()) : count(attacks_bb & wr.getBits());
            int32_t num_queen_attacks = is_white ? count(attacks_bb & bq.getBits()) : count(attacks_bb & wq.getBits());
            int32_t num_king_attacks = is_white ? count(attacks_bb & bk.getBits()) : count(attacks_bb & wk.getBits());

            eval_array[is_white ? 0 : 1] += threats[j][0] * num_pawn_attacks;
            eval_array[is_white ? 0 : 1] += threats[j][1] * num_knight_attacks;
            eval_array[is_white ? 0 : 1] += threats[j][2] * num_bishop_attacks;
            eval_array[is_white ? 0 : 1] += threats[j][3] * num_rook_attacks;
            eval_array[is_white ? 0 : 1] += threats[j][4] * num_queen_attacks;
            eval_array[is_white ? 0 : 1] += threats[j][5] * num_king_attacks;

        }

    }

    // Rooks on open files
    /*
    if (num_w_rooks_on_op_file == 1) eval_array[0] += rook_open_file[0];
    if (num_w_rooks_on_op_file == 2) eval_array[0] += rook_open_file[1];
    if (num_b_rooks_on_op_file == 1) eval_array[1] += rook_open_file[0];
    if (num_b_rooks_on_op_file == 2) eval_array[1] += rook_open_file[1];
    */

    // Bishop Pair
    if (wb.count() == 2) eval_array[0] += bishop_pair;
    if (bb.count() == 2) eval_array[1] += bishop_pair;

    int32_t stm = board.sideToMove() == Color::WHITE ? 0 : 1;
    int32_t score = eval_array[stm] - eval_array[stm^1];
    int32_t mg_score = (int32_t)unpack_mg(score);
    int32_t eg_score = (int32_t)unpack_eg(score);
    int32_t mg_phase = phase;
    if (mg_phase > 24) mg_phase = 24;
    int32_t eg_phase = 24 - mg_phase; 

    // Evaluation tapering, that is, interpolating mg and eg values depending on how many pieces
    // there are on the board. See here for more information: https://www.chessprogramming.org/Tapered_Eval
    return ((mg_score * mg_phase + eg_score * eg_phase) / 24);
}
