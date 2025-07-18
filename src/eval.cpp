// error 0.0762988

#include <stdint.h>

#include "chess.hpp"
#include "packing.hpp"
#include "eval.hpp"
#include "defaults.hpp"
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

// 11.
// Rook on semi open file bonus
// It is usually better for rooks to be in semi open files in the middlegame to control
// more squares

const int32_t PSQT[6][64] = {
    {
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
        S(44, 184), S(74, 197), S(69, 180), S(79, 144), S(79, 119), S(79, 130), S(48, 152), S(34, 159),
        S(83, 159), S(92, 181), S(91, 167), S(103, 150), S(115, 111), S(153, 113), S(131, 157), S(103, 135),
        S(80, 129), S(97, 137), S(93, 125), S(76, 117), S(93, 103), S(98, 106), S(85, 121), S(80, 99),
        S(75, 106), S(82, 120), S(89, 105), S(87, 113), S(81, 103), S(90, 96), S(73, 110), S(70, 87),
        S(89, 98), S(101, 107), S(89, 103), S(62, 113), S(81, 110), S(86, 101), S(95, 102), S(81, 83),
        S(82, 96), S(111, 100), S(99, 96), S(80, 108), S(74, 120), S(93, 100), S(103, 97), S(74, 77),
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
    },
    {
        S(13, 81), S(21, 142), S(58, 129), S(87, 121), S(122, 117), S(47, 102), S(44, 135), S(62, 58),
        S(100, 143), S(105, 126), S(83, 125), S(102, 125), S(97, 116), S(120, 107), S(108, 118), S(125, 124),
        S(108, 118), S(103, 122), S(154, 168), S(161, 171), S(174, 163), S(196, 147), S(110, 117), S(126, 110),
        S(118, 132), S(105, 134), S(154, 177), S(171, 181), S(153, 181), S(175, 178), S(108, 137), S(146, 122),
        S(107, 133), S(94, 130), S(138, 178), S(137, 181), S(145, 184), S(143, 169), S(112, 126), S(117, 124),
        S(90, 120), S(82, 125), S(122, 161), S(129, 174), S(140, 171), S(128, 155), S(99, 117), S(107, 119),
        S(104, 144), S(90, 125), S(73, 122), S(89, 122), S(88, 120), S(83, 118), S(103, 114), S(127, 152),
        S(86, 137), S(114, 135), S(80, 122), S(93, 121), S(97, 122), S(100, 112), S(115, 143), S(114, 125)
    },
    {
        S(112, 170), S(92, 176), S(96, 167), S(58, 178), S(75, 172), S(81, 166), S(115, 163), S(81, 165),
        S(104, 163), S(118, 163), S(115, 163), S(108, 162), S(109, 157), S(120, 158), S(99, 171), S(104, 163),
        S(125, 175), S(137, 165), S(133, 164), S(135, 158), S(130, 162), S(152, 166), S(142, 165), S(130, 177),
        S(115, 172), S(122, 170), S(127, 166), S(144, 175), S(135, 169), S(131, 170), S(122, 167), S(119, 168),
        S(122, 169), S(111, 172), S(125, 168), S(137, 170), S(136, 167), S(130, 162), S(122, 165), S(130, 159),
        S(124, 169), S(136, 165), S(133, 165), S(132, 165), S(135, 168), S(136, 161), S(140, 157), S(145, 159),
        S(143, 169), S(139, 154), S(142, 150), S(129, 160), S(136, 159), S(140, 154), S(154, 159), S(147, 152),
        S(136, 158), S(147, 164), S(135, 160), S(124, 165), S(129, 163), S(127, 171), S(137, 156), S(155, 140)
    },
    {
        S(158, 303), S(150, 308), S(154, 315), S(153, 313), S(156, 306), S(180, 300), S(161, 305), S(181, 299),
        S(156, 299), S(157, 306), S(171, 311), S(186, 302), S(171, 304), S(196, 289), S(192, 286), S(205, 281),
        S(150, 295), S(174, 290), S(168, 294), S(171, 288), S(195, 280), S(203, 272), S(224, 269), S(196, 272),
        S(148, 298), S(159, 292), S(162, 297), S(168, 293), S(164, 284), S(176, 276), S(170, 280), S(171, 277),
        S(136, 297), S(131, 299), S(141, 297), S(151, 296), S(148, 295), S(142, 289), S(150, 282), S(147, 281),
        S(131, 298), S(130, 294), S(137, 294), S(141, 296), S(144, 292), S(146, 283), S(161, 269), S(149, 274),
        S(134, 293), S(138, 293), S(148, 294), S(150, 292), S(153, 287), S(152, 282), S(165, 273), S(142, 281),
        S(152, 301), S(149, 296), S(152, 300), S(158, 293), S(158, 290), S(149, 295), S(156, 286), S(154, 287)
    },
    {
        S(327, 564), S(320, 570), S(338, 585), S(365, 571), S(349, 573), S(360, 572), S(404, 524), S(360, 558),
        S(348, 551), S(335, 562), S(331, 593), S(319, 611), S(312, 629), S(359, 572), S(363, 567), S(389, 571),
        S(358, 549), S(351, 555), S(353, 576), S(351, 582), S(359, 585), S(384, 571), S(393, 548), S(378, 562),
        S(341, 569), S(349, 568), S(345, 574), S(344, 587), S(344, 589), S(351, 581), S(359, 584), S(357, 575),
        S(351, 559), S(340, 574), S(344, 570), S(348, 581), S(345, 579), S(347, 570), S(352, 566), S(357, 571),
        S(346, 545), S(353, 552), S(347, 565), S(344, 560), S(347, 564), S(349, 559), S(362, 541), S(357, 549),
        S(350, 548), S(352, 544), S(359, 538), S(354, 550), S(355, 551), S(354, 522), S(361, 502), S(373, 496),
        S(344, 552), S(345, 543), S(349, 544), S(353, 556), S(351, 535), S(331, 534), S(343, 524), S(356, 520)
    },
    {
        S(74, -119), S(70, -68), S(103, -53), S(-21, -6), S(-39, 4), S(-31, 5), S(42, -14), S(168, -114),
        S(-58, -28), S(43, -7), S(20, 9), S(108, 2), S(21, 40), S(11, 47), S(31, 30), S(-17, 8),
        S(-77, -19), S(77, -6), S(37, 20), S(20, 36), S(22, 60), S(61, 45), S(25, 32), S(-37, 10),
        S(-42, -27), S(-3, 0), S(-7, 26), S(-44, 49), S(-70, 76), S(-49, 60), S(-65, 38), S(-126, 23),
        S(-58, -29), S(-13, -5), S(-23, 21), S(-50, 42), S(-70, 74), S(-39, 50), S(-58, 33), S(-131, 22),
        S(-26, -33), S(21, -13), S(-6, 8), S(-15, 20), S(-25, 52), S(-18, 40), S(-1, 19), S(-52, 10),
        S(29, -45), S(30, -20), S(22, -9), S(-1, 0), S(-18, 35), S(-4, 26), S(25, 8), S(4, -7),
        S(14, -79), S(43, -59), S(26, -39), S(-38, -21), S(-5, -6), S(-33, 6), S(16, -17), S(0, -44)
    },
};


const int32_t mobilities[5][28] = {

    {
        S(0, 0), S(0, 0), S(115, 195), S(139, 193), S(163, 225), S(0, 0), S(197, 237), S(0, 0), S(173, 207), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(119, 142), S(134, 138), S(139, 169), S(152, 178), S(157, 187), S(168, 202), S(173, 209), S(179, 217), S(180, 221), S(183, 227), S(183, 225), S(185, 223), S(203, 220), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(0, 0), S(198, 304), S(201, 322), S(206, 330), S(209, 336), S(209, 340), S(209, 345), S(212, 347), S(215, 351), S(217, 356), S(218, 359), S(219, 365), S(223, 369), S(221, 375), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(0, 0), S(0, 0), S(442, 348), S(434, 418), S(459, 472), S(461, 494), S(458, 555), S(460, 569), S(458, 590), S(460, 597), S(461, 609), S(463, 620), S(465, 622), S(467, 627), S(467, 636), S(466, 642), S(467, 647), S(466, 653), S(467, 657), S(467, 665), S(469, 664), S(470, 665), S(479, 660), S(476, 661), S(491, 655), S(525, 634), S(573, 607),
    },
    {
        S(0, 0), S(0, 0), S(0, 0), S(47, -9), S(52, -18), S(36, 0), S(27, 0), S(24, -5), S(20, -3), S(15, -3), S(14, -4), S(6, 1), S(5, -1), S(-3, 1), S(-12, 3), S(-22, 5), S(-35, 6), S(-51, 6), S(-64, 6), S(-79, 6), S(-79, 2), S(-84, 0), S(-91, -2), S(-99, -7), S(-115, -7), S(-99, -19), S(-113, -17), S(-89, -31),
    },
};


const int32_t bishop_pair = S(19, 59);


const int32_t passed_pawns[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(44, 184), S(74, 197), S(69, 180), S(79, 144), S(79, 119), S(79, 130), S(48, 152), S(34, 159),
    S(18, 144), S(33, 150), S(21, 115), S(8, 75), S(10, 86), S(13, 103), S(-17, 114), S(-40, 143),
    S(15, 79), S(18, 77), S(19, 53), S(15, 48), S(4, 47), S(10, 54), S(0, 71), S(-11, 79),
    S(5, 44), S(0, 38), S(-11, 29), S(-5, 23), S(-12, 26), S(-6, 30), S(3, 41), S(1, 39),
    S(0, 8), S(-8, 17), S(-17, 13), S(-17, 7), S(-13, 6), S(-6, 5), S(8, 21), S(14, 6),
    S(-7, 7), S(-3, 12), S(-16, 12), S(-15, 6), S(1, -7), S(2, -1), S(20, 3), S(7, 6),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};

const int32_t inner_king_zone_attacks[4] = {
    S(10, -6), S(19, -4), S(20, -7), S(13, 7),
};


const int32_t outer_king_zone_attacks[4] = {
    S(0, 1), S(0, 0), S(4, -4), S(2, 1),
};


const int32_t doubled_pawn_penalty[8] = {
    S(-6, -39), S(-4, -26), S(-11, -23), S(-9, -10), S(-8, -20), S(-11, -21), S(-5, -25), S(-13, -35),
};


const int32_t pawn_storm[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(22, -60), S(-35, -53), S(-5, -91), S(-17, -48), S(-61, 49), S(-51, -10), S(-3, -9), S(-35, 1),
    S(-17, -33), S(-20, -41), S(10, -52), S(-1, -27), S(-11, 17), S(-30, 0), S(-42, -8), S(-44, 4),
    S(-29, -11), S(-30, -13), S(-21, -14), S(-2, -15), S(-8, 13), S(-17, 3), S(-33, 10), S(-45, 16),
    S(-31, -5), S(-24, -5), S(-20, -3), S(-7, -9), S(-5, 6), S(-21, 7), S(-27, 8), S(-41, 16),
    S(-46, 3), S(-45, 4), S(-24, 0), S(7, -5), S(-11, 1), S(-22, 3), S(-57, 17), S(-59, 23),
    S(-37, 8), S(-55, 10), S(-44, 9), S(-21, 3), S(-27, 4), S(-39, 8), S(-70, 27), S(-53, 37),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};

const int32_t isolated_pawns[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(49, -26), S(111, -106), S(52, -8), S(53, -7), S(17, 3), S(3, 26), S(-26, 21), S(0, 2),
    S(11, -17), S(13, -48), S(3, -25), S(5, -12), S(2, -17), S(-3, -8), S(22, -28), S(7, -25),
    S(8, -9), S(-1, -27), S(-7, -17), S(-6, -21), S(-5, -23), S(4, -20), S(19, -27), S(4, -7),
    S(-2, 1), S(-5, -12), S(-21, -6), S(-22, -19), S(-16, -19), S(-17, -6), S(-15, -9), S(-9, 1),
    S(-11, -2), S(-12, -15), S(-22, -10), S(-13, -17), S(-29, -15), S(-21, -7), S(-23, -15), S(-21, 0),
    S(-8, -3), S(-14, -8), S(-3, -10), S(-27, -6), S(-27, -10), S(-9, -5), S(-2, -16), S(-12, 3),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};

const int32_t threats[6][6] = {

    {
        S(3, -9), S(11, -44), S(26, -30), S(-19, -27), S(-10, -62), S(-87, 2),
    },
    {
        S(-10, 4), S(0, 0), S(20, 22), S(48, -6), S(22, -34), S(0, 0),
    },
    {
        S(-6, 5), S(15, 19), S(0, 0), S(30, 4), S(33, 68), S(0, 0),
    },
    {
        S(-16, 12), S(1, 14), S(9, 11), S(0, 0), S(45, 0), S(0, 0),
    },
    {
        S(-4, 5), S(0, 6), S(-3, 26), S(2, -1), S(0, 0), S(0, 0),
    },
    {
        S(37, 33), S(-40, 8), S(-19, 21), S(-15, -2), S(0, 0), S(0, 0),
    },
};


const int32_t rook_semi_open[2] = {
    S(21, 9), S(54, 4),
};


// For a tapered evaluation
const int32_t game_phase_increment[6] = {0, 1, 1, 2, 4, 0};

// This is our HCE evaluation function. 
int32_t evaluate(const chess::Board& board) {

    int32_t eval_array[2] = {0,0};
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

    // Rooks on semi open files
    int32_t num_w_rooks_on_semi_op_file = 0;
    int32_t num_b_rooks_on_semi_op_file = 0;

    // A fast way of getting all the pieces 
    for (int32_t i = 0; i < 12; i++){        
        chess::Bitboard curr_bb = all[i];
        while (!curr_bb.empty()) {
            int32_t sq = curr_bb.pop();
            bool is_white = i < 6;
            int32_t j = is_white ? i : i-6;

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
                        attacks = __builtin_popcountll(attacks_bb);
                        break;
                    // bishops
                    case 2:
                        attacks_bb = chess::attacks::bishop(static_cast<chess::Square>(sq), board.occ()).getBits();
                        attacks = __builtin_popcountll(attacks_bb);
                        break;
                    // rooks
                    case 3:
                        // Rook on semi-open file
                        if ((is_white ? (WHITE_AHEAD_MASK[sq] & wp) : (BLACK_AHEAD_MASK[sq] & bp)) == 0ull){
                            if (is_white) num_w_rooks_on_semi_op_file++;
                            else num_b_rooks_on_semi_op_file++;
                        }

                        attacks_bb = chess::attacks::rook(static_cast<chess::Square>(sq), board.occ()).getBits();
                        attacks = __builtin_popcountll(attacks_bb);
                        break;
                    // queens
                    case 4:
                        attacks_bb = chess::attacks::queen(static_cast<chess::Square>(sq), board.occ()).getBits();
                        attacks = __builtin_popcountll(attacks_bb);
                        break;
                    // King Virtual Mobility
                    case 5:
                        attacks = chess::attacks::queen(static_cast<chess::Square>(sq), board.occ()).count();
                        break;

                    default:
                        break;
                }

                // Mobilities
                eval_array[is_white ? 0 : 1] += mobilities[j-1][attacks];
                
                // Non king non pawn pieces
                if (j < 5){
                    
                    // King zone attacks
                    eval_array[is_white ? 0 : 1] += inner_king_zone_attacks[j-1]  * __builtin_popcountll((is_white ? black_king_inner_sq_mask : white_king_inner_sq_mask) & attacks_bb); 
                    eval_array[is_white ? 0 : 1] += outer_king_zone_attacks[j-1]  * __builtin_popcountll((is_white ? black_king_2_sq_mask : white_king_2_sq_mask) & attacks_bb); 
                }
            }

            // Pawns
            else {
                // Doubled pawns
                // Note that for pawns to be considered "doubled", they need not be directly in front
                // of another pawn
                uint64_t front_mask = is_white ? WHITE_AHEAD_MASK[sq] : BLACK_AHEAD_MASK[sq];
                uint64_t our_pawn_bb = is_white ? wp.getBits() : bp.getBits();
                if (front_mask & our_pawn_bb){
                    eval_array[is_white ? 0 : 1] += doubled_pawn_penalty[is_white ? 7 - sq % 8 : sq % 8];
                }

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
            int32_t num_pawn_attacks = is_white ? __builtin_popcountll(attacks_bb & bp.getBits()) : __builtin_popcountll(attacks_bb & wp.getBits());
            int32_t num_knight_attacks = is_white ? __builtin_popcountll(attacks_bb & bn.getBits()) : __builtin_popcountll(attacks_bb & wn.getBits());
            int32_t num_bishop_attacks = is_white ? __builtin_popcountll(attacks_bb & bb.getBits()) : __builtin_popcountll(attacks_bb & wb.getBits());
            int32_t num_rook_attacks = is_white ? __builtin_popcountll(attacks_bb & br.getBits()) : __builtin_popcountll(attacks_bb & wr.getBits());
            int32_t num_queen_attacks = is_white ? __builtin_popcountll(attacks_bb & bq.getBits()) : __builtin_popcountll(attacks_bb & wq.getBits());
            int32_t num_king_attacks = is_white ? __builtin_popcountll(attacks_bb & bk.getBits()) : __builtin_popcountll(attacks_bb & wk.getBits());

            eval_array[is_white ? 0 : 1] += threats[j][0] * num_pawn_attacks;
            eval_array[is_white ? 0 : 1] += threats[j][1] * num_knight_attacks;
            eval_array[is_white ? 0 : 1] += threats[j][2] * num_bishop_attacks;
            eval_array[is_white ? 0 : 1] += threats[j][3] * num_rook_attacks;
            eval_array[is_white ? 0 : 1] += threats[j][4] * num_queen_attacks;
            eval_array[is_white ? 0 : 1] += threats[j][5] * num_king_attacks;

        }

    }

    // Bishop Pair
    if (wb.count() == 2) eval_array[0] += bishop_pair; 
    if (bb.count() == 2) eval_array[1] += bishop_pair; 

    // Rooks on semi-open files
    if (num_w_rooks_on_semi_op_file == 1) eval_array[0] += rook_semi_open[0];
    if (num_w_rooks_on_semi_op_file == 2) eval_array[0] += rook_semi_open[1];
    if (num_b_rooks_on_semi_op_file == 1) eval_array[1] += rook_semi_open[0];
    if (num_b_rooks_on_semi_op_file == 2) eval_array[1] += rook_semi_open[1];


    int32_t stm = board.sideToMove() == Color::WHITE ? 0 : 1;
    int32_t score = eval_array[stm] - eval_array[stm^1];
    int32_t mg_score = (int32_t)unpack_mg(score);
    int32_t eg_score = (int32_t)unpack_eg(score);
    int32_t mg_phase = phase;
    if (mg_phase > 24) mg_phase = 24;
    int32_t eg_phase = 24 - mg_phase; 

    // Evaluation tapering, that is, interpolating mg and eg values depending on how many pieces
    // there are on the board. See here for more information: https://www.chessprogramming.org/Tapered_Eval
    return tempo.current + ((mg_score * mg_phase + eg_score * eg_phase) / 24);
}
