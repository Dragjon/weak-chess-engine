// error 0.0764196

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

const int32_t PSQT[6][64] = {
    {
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
        S(40, 183), S(72, 197), S(64, 180), S(71, 145), S(74, 119), S(73, 130), S(46, 152), S(30, 159),
        S(76, 160), S(90, 181), S(85, 168), S(89, 152), S(106, 111), S(140, 116), S(128, 157), S(97, 137),
        S(76, 129), S(94, 138), S(90, 125), S(62, 120), S(87, 104), S(87, 109), S(82, 122), S(75, 101),
        S(72, 106), S(80, 121), S(87, 106), S(74, 116), S(77, 104), S(80, 99), S(69, 110), S(66, 88),
        S(86, 98), S(98, 108), S(87, 103), S(51, 116), S(78, 111), S(77, 104), S(91, 103), S(77, 84),
        S(80, 97), S(110, 100), S(98, 96), S(70, 112), S(71, 121), S(86, 103), S(100, 97), S(71, 78),
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
    },
    {
        S(14, 82), S(22, 142), S(62, 128), S(89, 120), S(123, 116), S(48, 101), S(44, 135), S(65, 59),
        S(102, 142), S(105, 125), S(83, 124), S(102, 124), S(97, 115), S(121, 106), S(107, 117), S(126, 124),
        S(109, 117), S(105, 121), S(155, 168), S(161, 171), S(175, 163), S(196, 147), S(111, 116), S(124, 110),
        S(119, 131), S(104, 133), S(154, 177), S(171, 181), S(153, 182), S(174, 178), S(106, 137), S(144, 122),
        S(106, 132), S(93, 129), S(138, 178), S(137, 181), S(145, 184), S(143, 169), S(111, 125), S(115, 123),
        S(90, 118), S(80, 124), S(122, 161), S(129, 174), S(140, 171), S(127, 155), S(97, 116), S(106, 118),
        S(103, 144), S(89, 124), S(72, 121), S(88, 121), S(87, 119), S(83, 116), S(101, 114), S(125, 152),
        S(83, 139), S(114, 134), S(80, 121), S(93, 120), S(97, 121), S(99, 112), S(114, 143), S(112, 126)
    },
    {
        S(112, 170), S(92, 175), S(96, 167), S(58, 178), S(75, 172), S(81, 165), S(117, 162), S(80, 165),
        S(105, 163), S(116, 163), S(115, 163), S(108, 162), S(109, 157), S(122, 158), S(98, 171), S(103, 162),
        S(125, 174), S(138, 165), S(133, 164), S(136, 158), S(129, 162), S(154, 166), S(142, 165), S(130, 176),
        S(116, 172), S(121, 170), S(128, 166), S(145, 175), S(136, 169), S(131, 170), S(120, 167), S(118, 168),
        S(122, 169), S(112, 171), S(124, 168), S(137, 171), S(136, 167), S(130, 163), S(121, 166), S(128, 160),
        S(126, 168), S(135, 165), S(133, 165), S(132, 165), S(134, 168), S(136, 161), S(138, 157), S(144, 159),
        S(144, 169), S(139, 154), S(141, 150), S(129, 160), S(136, 158), S(141, 154), S(154, 159), S(146, 152),
        S(134, 158), S(146, 164), S(136, 160), S(125, 165), S(130, 162), S(127, 171), S(136, 156), S(152, 141)
    },
    {
        S(168, 307), S(156, 313), S(159, 321), S(156, 319), S(160, 312), S(185, 306), S(168, 310), S(190, 304),
        S(165, 303), S(157, 312), S(170, 317), S(184, 308), S(171, 310), S(196, 296), S(194, 292), S(214, 285),
        S(157, 299), S(176, 296), S(167, 300), S(169, 295), S(194, 286), S(198, 280), S(225, 275), S(201, 277),
        S(154, 304), S(161, 298), S(163, 303), S(169, 299), S(166, 290), S(176, 283), S(173, 286), S(176, 284),
        S(140, 303), S(138, 303), S(147, 303), S(157, 301), S(155, 300), S(147, 295), S(159, 287), S(153, 287),
        S(135, 303), S(140, 297), S(148, 297), S(151, 300), S(154, 296), S(154, 287), S(173, 272), S(158, 278),
        S(137, 297), S(146, 296), S(161, 295), S(162, 295), S(164, 289), S(160, 284), S(174, 275), S(148, 285),
        S(153, 301), S(155, 298), S(164, 301), S(169, 296), S(168, 293), S(155, 296), S(165, 287), S(157, 287)
    },
    {
        S(328, 561), S(319, 570), S(336, 585), S(362, 572), S(345, 574), S(356, 573), S(402, 523), S(362, 556),
        S(350, 549), S(333, 562), S(329, 594), S(317, 612), S(311, 629), S(358, 573), S(362, 567), S(390, 569),
        S(358, 549), S(350, 555), S(350, 578), S(348, 583), S(355, 589), S(382, 573), S(391, 549), S(377, 563),
        S(340, 569), S(348, 568), S(343, 575), S(342, 589), S(341, 592), S(349, 584), S(356, 586), S(355, 577),
        S(349, 560), S(337, 575), S(340, 573), S(346, 583), S(342, 582), S(344, 572), S(349, 568), S(355, 571),
        S(345, 545), S(351, 554), S(345, 566), S(341, 562), S(344, 567), S(347, 560), S(359, 542), S(355, 550),
        S(350, 545), S(350, 545), S(358, 540), S(353, 549), S(353, 550), S(353, 521), S(360, 502), S(375, 492),
        S(343, 550), S(345, 542), S(350, 542), S(354, 555), S(352, 532), S(331, 533), S(341, 524), S(354, 519)
    },
    {
        S(73, -118), S(70, -67), S(104, -53), S(-21, -6), S(-40, 5), S(-34, 6), S(40, -13), S(166, -114),
        S(-57, -28), S(44, -7), S(20, 10), S(110, 2), S(21, 40), S(11, 48), S(30, 30), S(-18, 8),
        S(-76, -19), S(79, -6), S(37, 20), S(21, 36), S(22, 60), S(63, 44), S(26, 31), S(-35, 10),
        S(-42, -27), S(-3, 0), S(-8, 26), S(-44, 49), S(-70, 76), S(-49, 60), S(-64, 37), S(-125, 22),
        S(-60, -29), S(-14, -5), S(-23, 21), S(-51, 43), S(-70, 73), S(-40, 51), S(-58, 33), S(-131, 21),
        S(-29, -33), S(18, -13), S(-8, 8), S(-16, 20), S(-28, 52), S(-21, 40), S(-3, 20), S(-54, 10),
        S(28, -44), S(29, -20), S(20, -8), S(0, 0), S(-20, 35), S(-7, 26), S(24, 8), S(4, -8),
        S(13, -80), S(43, -59), S(26, -39), S(-35, -21), S(-4, -5), S(-32, 6), S(17, -17), S(0, -44)
    },
};



const int32_t mobilities[5][28] = {

    {
        S(0, 0), S(0, 0), S(114, 194), S(140, 193), S(163, 226), S(0, 0), S(198, 239), S(0, 0), S(173, 208), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(118, 143), S(133, 139), S(138, 169), S(152, 179), S(157, 188), S(167, 202), S(172, 209), S(179, 218), S(180, 222), S(183, 228), S(182, 225), S(185, 223), S(203, 220), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(0, 0), S(195, 306), S(199, 322), S(205, 330), S(209, 336), S(209, 340), S(210, 346), S(215, 348), S(220, 352), S(224, 357), S(228, 361), S(231, 368), S(237, 372), S(238, 378), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(0, 0), S(0, 0), S(445, 344), S(432, 421), S(456, 476), S(456, 501), S(454, 561), S(457, 573), S(455, 593), S(458, 599), S(459, 611), S(461, 622), S(463, 624), S(465, 630), S(465, 638), S(465, 644), S(466, 649), S(465, 655), S(466, 658), S(467, 666), S(469, 665), S(470, 666), S(479, 660), S(478, 660), S(493, 654), S(528, 631), S(573, 606),
    },
    {
        S(0, 0), S(0, 0), S(0, 0), S(46, -10), S(52, -17), S(34, 0), S(27, 0), S(24, -5), S(20, -3), S(16, -3), S(15, -4), S(7, 1), S(6, -1), S(-1, 1), S(-10, 3), S(-21, 5), S(-34, 6), S(-50, 6), S(-63, 6), S(-78, 6), S(-79, 2), S(-84, 0), S(-91, -2), S(-99, -6), S(-114, -7), S(-99, -18), S(-115, -17), S(-90, -30),
    },
};


const int32_t bishop_pair = S(19, 59);


const int32_t passed_pawns[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(40, 183), S(72, 197), S(64, 180), S(71, 145), S(74, 119), S(73, 130), S(46, 152), S(30, 159),
    S(19, 143), S(31, 149), S(19, 114), S(5, 75), S(11, 86), S(15, 101), S(-15, 113), S(-38, 142),
    S(15, 79), S(19, 76), S(17, 54), S(15, 48), S(4, 46), S(12, 53), S(1, 71), S(-9, 79),
    S(5, 44), S(1, 38), S(-11, 30), S(-4, 23), S(-11, 25), S(-4, 29), S(5, 40), S(2, 39),
    S(1, 8), S(-5, 16), S(-16, 13), S(-17, 7), S(-12, 6), S(-2, 4), S(12, 20), S(16, 6),
    S(-5, 7), S(-1, 12), S(-15, 12), S(-13, 5), S(0, -7), S(5, -3), S(23, 3), S(9, 6),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};


const int32_t inner_king_zone_attacks[4] = {
    S(10, -6), S(19, -4), S(21, -7), S(13, 6),
};


const int32_t outer_king_zone_attacks[4] = {
    S(0, 1), S(0, 0), S(3, -4), S(2, 0),
};


const int32_t doubled_pawn_penalty[8] = {
    S(0, -41), S(0, -27), S(-4, -24), S(-4, -10), S(-3, -20), S(-5, -23), S(-2, -25), S(-7, -36),
};


const int32_t pawn_storm[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(16, -59), S(-38, -52), S(-5, -91), S(-13, -50), S(-57, 49), S(-43, -11), S(-4, -9), S(-41, 2),
    S(-22, -32), S(-22, -41), S(10, -52), S(8, -30), S(-8, 17), S(-25, -1), S(-44, -8), S(-53, 5),
    S(-35, -9), S(-30, -13), S(-22, -14), S(6, -18), S(-6, 13), S(-11, 2), S(-34, 10), S(-52, 18),
    S(-37, -4), S(-25, -5), S(-21, -3), S(4, -12), S(-3, 6), S(-14, 6), S(-28, 8), S(-49, 17),
    S(-51, 5), S(-45, 4), S(-24, 0), S(18, -8), S(-10, 1), S(-16, 1), S(-58, 17), S(-66, 24),
    S(-42, 9), S(-56, 10), S(-45, 9), S(-11, 0), S(-26, 4), S(-35, 7), S(-71, 27), S(-60, 38),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};


const int32_t isolated_pawns[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(47, -25), S(110, -106), S(52, -7), S(52, -6), S(21, 3), S(5, 26), S(-26, 21), S(0, 1),
    S(13, -17), S(12, -48), S(2, -24), S(4, -12), S(2, -17), S(-2, -8), S(20, -27), S(8, -24),
    S(8, -9), S(-2, -27), S(-8, -17), S(-7, -21), S(-6, -23), S(5, -20), S(17, -26), S(4, -7),
    S(-2, 1), S(-8, -11), S(-22, -6), S(-23, -19), S(-16, -19), S(-17, -7), S(-17, -8), S(-10, 2),
    S(-12, -2), S(-14, -15), S(-24, -10), S(-14, -16), S(-30, -15), S(-20, -8), S(-26, -14), S(-22, 0),
    S(-8, -3), S(-15, -8), S(-4, -10), S(-28, -6), S(-28, -10), S(-9, -5), S(-3, -16), S(-13, 3),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};


const int32_t threats[6][6] = {
    {
        S(3, -9), S(11, -44), S(25, -30), S(-21, -26), S(-11, -61), S(-86, 2),
    },
    {
        S(-10, 4), S(0, 0), S(20, 22), S(47, -5), S(22, -33), S(0, 0),
    },
    {
        S(-6, 5), S(15, 19), S(0, 0), S(30, 4), S(33, 68), S(0, 0),
    },
    {
        S(-5, 12), S(8, 15), S(16, 12), S(0, 0), S(52, 0), S(0, 0),
    },
    {
        S(-4, 5), S(0, 5), S(-3, 26), S(2, -2), S(0, 0), S(0, 0),
    },
    {
        S(36, 33), S(-41, 8), S(-20, 21), S(-14, -2), S(0, 0), S(0, 0),
    },
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
