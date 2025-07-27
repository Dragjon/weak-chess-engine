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

// 12.
// Pawn phalanxes
// A really simple eval term. Gives bonuses to pawns that are adjacent to each other
// and indexed by [rank]. This can be useful for endgames where connected passers 
// are really strong.

// 13.
// Defenders
// Defenders are indexed by [our piece][our other piece]. It gives bonuses to defended
// pieces, which could help ensure that most pieces are protected.

const int32_t PSQT[6][64] = {
    {
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
        S(51, 179), S(80, 192), S(72, 178), S(84, 143), S(82, 121), S(80, 131), S(54, 151), S(36, 159),
        S(86, 156), S(92, 180), S(91, 161), S(106, 142), S(114, 104), S(156, 109), S(136, 155), S(112, 131),
        S(77, 122), S(94, 129), S(84, 117), S(71, 109), S(91, 96), S(89, 98), S(84, 115), S(81, 94),
        S(75, 97), S(76, 114), S(82, 96), S(76, 103), S(74, 94), S(79, 87), S(71, 102), S(69, 78),
        S(77, 88), S(78, 98), S(70, 94), S(44, 103), S(63, 100), S(67, 91), S(76, 90), S(68, 72),
        S(81, 95), S(109, 102), S(93, 95), S(73, 104), S(73, 116), S(90, 99), S(103, 97), S(70, 74),
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
    },
    {
        S(19, 78), S(27, 138), S(56, 125), S(83, 117), S(117, 115), S(47, 98), S(51, 134), S(60, 58),
        S(105, 141), S(104, 122), S(87, 125), S(104, 126), S(99, 115), S(118, 108), S(103, 116), S(128, 122),
        S(106, 114), S(108, 122), S(155, 167), S(162, 168), S(173, 163), S(198, 146), S(113, 118), S(120, 107),
        S(115, 127), S(108, 131), S(156, 173), S(169, 179), S(152, 179), S(176, 175), S(110, 135), S(143, 118),
        S(104, 126), S(99, 125), S(140, 173), S(137, 175), S(146, 179), S(144, 162), S(120, 120), S(116, 116),
        S(84, 112), S(80, 119), S(121, 154), S(129, 168), S(137, 164), S(124, 148), S(99, 108), S(101, 110),
        S(106, 140), S(85, 119), S(73, 120), S(83, 120), S(85, 116), S(81, 114), S(97, 107), S(128, 147),
        S(78, 134), S(113, 131), S(75, 116), S(91, 115), S(96, 113), S(98, 101), S(118, 132), S(109, 121)
    },
    {
        S(120, 170), S(99, 174), S(97, 167), S(62, 176), S(76, 172), S(87, 165), S(123, 161), S(91, 163),
        S(115, 163), S(123, 163), S(123, 162), S(113, 161), S(114, 156), S(122, 158), S(102, 170), S(106, 163),
        S(129, 173), S(144, 164), S(134, 163), S(138, 158), S(133, 161), S(156, 166), S(144, 165), S(134, 176),
        S(120, 170), S(125, 167), S(132, 165), S(149, 174), S(139, 167), S(133, 168), S(125, 165), S(123, 166),
        S(128, 166), S(115, 169), S(127, 165), S(140, 168), S(138, 165), S(132, 160), S(124, 163), S(138, 156),
        S(128, 166), S(140, 163), S(136, 161), S(130, 162), S(136, 165), S(138, 159), S(146, 154), S(149, 155),
        S(145, 166), S(143, 149), S(141, 148), S(129, 157), S(134, 156), S(138, 153), S(151, 157), S(150, 150),
        S(141, 155), S(150, 161), S(140, 158), S(129, 162), S(138, 159), S(132, 166), S(140, 154), S(161, 139)
    },
    {
        S(171, 302), S(161, 307), S(163, 315), S(159, 313), S(166, 305), S(190, 301), S(172, 305), S(192, 299),
        S(163, 301), S(164, 307), S(174, 313), S(190, 303), S(175, 305), S(197, 293), S(199, 289), S(212, 283),
        S(158, 297), S(182, 292), S(172, 297), S(175, 291), S(200, 283), S(204, 277), S(230, 274), S(201, 275),
        S(157, 301), S(166, 296), S(168, 301), S(173, 297), S(169, 288), S(180, 281), S(175, 285), S(177, 282),
        S(144, 301), S(140, 304), S(148, 303), S(156, 302), S(155, 300), S(150, 295), S(157, 289), S(153, 286),
        S(137, 303), S(138, 300), S(142, 302), S(142, 305), S(149, 301), S(152, 290), S(169, 277), S(156, 280),
        S(139, 297), S(143, 299), S(150, 301), S(152, 300), S(155, 294), S(162, 285), S(171, 277), S(150, 284),
        S(147, 301), S(146, 298), S(147, 301), S(151, 295), S(151, 292), S(150, 293), S(157, 287), S(150, 288)
    },
    {
        S(315, 549), S(315, 553), S(331, 566), S(353, 555), S(339, 558), S(352, 553), S(399, 504), S(347, 542),
        S(333, 540), S(320, 553), S(316, 584), S(305, 599), S(293, 618), S(336, 566), S(347, 556), S(369, 560),
        S(338, 541), S(337, 546), S(334, 571), S(333, 578), S(338, 581), S(365, 566), S(372, 543), S(356, 553),
        S(323, 559), S(330, 561), S(327, 570), S(326, 586), S(324, 588), S(331, 579), S(338, 580), S(335, 572),
        S(330, 553), S(320, 570), S(324, 567), S(327, 585), S(324, 581), S(326, 573), S(330, 567), S(336, 566),
        S(327, 538), S(331, 552), S(326, 564), S(318, 564), S(322, 574), S(324, 569), S(340, 550), S(336, 550),
        S(326, 543), S(330, 542), S(334, 542), S(327, 557), S(328, 555), S(326, 541), S(336, 523), S(350, 505),
        S(313, 547), S(324, 528), S(322, 538), S(324, 545), S(325, 536), S(311, 536), S(322, 528), S(332, 517)
    },
    {
        S(78, -120), S(81, -71), S(115, -55), S(-9, -11), S(-24, 1), S(-31, 5), S(45, -14), S(171, -117),
        S(-60, -30), S(49, -10), S(29, 6), S(118, -1), S(27, 36), S(23, 43), S(40, 27), S(-5, 5),
        S(-74, -22), S(80, -8), S(37, 18), S(22, 33), S(22, 58), S(56, 44), S(24, 31), S(-35, 8),
        S(-43, -29), S(0, -2), S(-7, 23), S(-43, 47), S(-72, 74), S(-53, 59), S(-64, 36), S(-124, 20),
        S(-57, -32), S(-13, -7), S(-22, 18), S(-53, 40), S(-75, 71), S(-42, 49), S(-59, 31), S(-131, 19),
        S(-29, -35), S(16, -15), S(-9, 7), S(-18, 18), S(-29, 51), S(-26, 39), S(-10, 19), S(-56, 7),
        S(32, -50), S(31, -23), S(23, -12), S(2, -2), S(-17, 33), S(-3, 24), S(24, 2), S(5, -12),
        S(9, -79), S(37, -56), S(28, -39), S(-33, -21), S(-6, -1), S(-31, 6), S(12, -13), S(-5, -42)
    },
};


const int32_t mobilities[5][28] = {

    {
        S(0, 0), S(0, 0), S(129, 197), S(142, 197), S(174, 230), S(0, 0), S(203, 238), S(0, 0), S(182, 208), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(112, 144), S(131, 140), S(139, 172), S(152, 182), S(160, 191), S(171, 205), S(175, 212), S(182, 220), S(185, 224), S(188, 230), S(189, 227), S(190, 225), S(207, 222), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(0, 0), S(195, 309), S(201, 329), S(203, 338), S(207, 344), S(208, 348), S(209, 356), S(212, 358), S(216, 361), S(218, 366), S(221, 370), S(224, 374), S(229, 379), S(235, 380), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(0, 0), S(0, 0), S(481, 355), S(469, 441), S(496, 490), S(496, 521), S(492, 581), S(495, 593), S(493, 614), S(496, 621), S(498, 632), S(499, 643), S(501, 647), S(504, 651), S(503, 659), S(503, 665), S(504, 670), S(503, 675), S(503, 679), S(504, 686), S(506, 684), S(508, 683), S(515, 677), S(512, 678), S(528, 669), S(561, 647), S(608, 620),
    },
    {
        S(0, 0), S(0, 0), S(0, 0), S(53, -5), S(48, -15), S(37, 1), S(27, 0), S(24, -5), S(20, -3), S(16, -4), S(14, -3), S(6, 1), S(4, 0), S(-3, 1), S(-12, 4), S(-22, 6), S(-35, 7), S(-51, 7), S(-62, 8), S(-77, 7), S(-77, 3), S(-82, 0), S(-88, -1), S(-95, -6), S(-107, -6), S(-96, -18), S(-111, -16), S(-87, -29),
    },
};


const int32_t bishop_pair = S(19, 61);


const int32_t passed_pawns[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(51, 179), S(80, 192), S(72, 178), S(84, 143), S(82, 121), S(80, 131), S(54, 151), S(36, 159),
    S(16, 145), S(34, 146), S(22, 111), S(6, 75), S(13, 86), S(7, 101), S(-19, 109), S(-45, 143),
    S(13, 81), S(22, 78), S(24, 54), S(15, 49), S(1, 47), S(15, 55), S(3, 71), S(-10, 79),
    S(4, 46), S(4, 40), S(-4, 31), S(0, 24), S(-9, 28), S(-1, 33), S(5, 44), S(1, 42),
    S(-1, 10), S(-2, 20), S(-10, 14), S(-10, 8), S(-6, 8), S(0, 8), S(13, 26), S(14, 8),
    S(-7, 8), S(-1, 12), S(-11, 13), S(-6, 6), S(8, -7), S(5, 0), S(21, 4), S(8, 8),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};

const int32_t inner_king_zone_attacks[4] = {
    S(10, -6), S(18, -4), S(20, -7), S(13, 7),
};


const int32_t outer_king_zone_attacks[4] = {
    S(0, 0), S(0, 0), S(4, -3), S(2, 0),
};


const int32_t doubled_pawn_penalty[8] = {
    S(-4, -39), S(-1, -27), S(-3, -22), S(-1, -10), S(0, -20), S(-3, -20), S(-1, -25), S(-10, -34),
};


const int32_t pawn_storm[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(20, -59), S(-29, -53), S(0, -93), S(-18, -48), S(-55, 48), S(-48, -11), S(-1, -11), S(-32, 0),
    S(-17, -33), S(-18, -42), S(12, -52), S(0, -27), S(-9, 16), S(-34, -1), S(-44, -6), S(-45, 5),
    S(-28, -10), S(-29, -14), S(-21, -14), S(-4, -15), S(-10, 13), S(-21, 2), S(-35, 10), S(-50, 16),
    S(-32, -5), S(-25, -5), S(-24, -2), S(-8, -8), S(-8, 8), S(-23, 7), S(-32, 8), S(-43, 15),
    S(-42, 3), S(-39, 3), S(-21, 0), S(9, -3), S(-11, 3), S(-21, 3), S(-54, 18), S(-55, 23),
    S(-35, 7), S(-49, 9), S(-39, 8), S(-16, 1), S(-21, 3), S(-33, 6), S(-64, 28), S(-49, 36),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};

const int32_t isolated_pawns[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(36, -13), S(100, -94), S(41, 0), S(42, -2), S(6, 4), S(1, 26), S(-40, 26), S(-5, 5),
    S(11, -14), S(13, -42), S(1, -13), S(1, -3), S(-1, -8), S(-4, -1), S(20, -20), S(4, -20),
    S(13, -3), S(0, -21), S(0, -9), S(-1, -13), S(-4, -15), S(12, -13), S(21, -21), S(8, -1),
    S(3, 7), S(0, -8), S(-11, 0), S(-11, -11), S(-9, -12), S(-7, 0), S(-12, -5), S(-5, 8),
    S(1, 6), S(4, -8), S(-8, -3), S(1, -9), S(-12, -7), S(-2, 0), S(-6, -7), S(-6, 10),
    S(-5, -2), S(-16, -9), S(-4, -9), S(-23, -2), S(-21, -8), S(-8, -4), S(-6, -18), S(-8, 5),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};

const int32_t threats[6][6] = {

    {
        S(-5, -7), S(20, -39), S(28, -21), S(-4, -25), S(7, -64), S(-84, 4),
    },
    {
        S(-11, 5), S(0, 0), S(26, 27), S(55, 4), S(41, -35), S(0, 0),
    },
    {
        S(-6, 6), S(18, 25), S(0, 0), S(43, 13), S(54, 63), S(0, 0),
    },
    {
        S(-11, 11), S(7, 15), S(15, 12), S(0, 0), S(70, -4), S(0, 0),
    },
    {
        S(-4, 4), S(1, 5), S(-2, 26), S(3, 3), S(0, 0), S(0, 0),
    },
    {
        S(41, 32), S(-38, 11), S(-9, 21), S(-10, -1), S(0, 0), S(0, 0),
    },
};


const int32_t rook_semi_open[2] = {
    S(15, 6), S(41, -3),
};


const int32_t phalanx_pawns[8] = {
    S(0, 0), S(3, -1), S(22, 9), S(27, 18), S(55, 47), S(128, 169), S(-142, 417), S(0, 0),
};


const int32_t defenders[6][6] = {

    {
        S(17, 11), S(0, 15), S(-2, 5), S(-6, -4), S(-1, 2), S(-8, -16),
    },
    {
        S(-5, 1), S(0, 0), S(-1, 4), S(4, 0), S(-1, 13), S(1, 1),
    },
    {
        S(0, -2), S(8, 15), S(127, -10), S(-1, 7), S(2, 17), S(-3, 0),
    },
    {
        S(0, -6), S(8, 1), S(5, 6), S(5, 3), S(6, 16), S(16, -9),
    },
    {
        S(-1, -3), S(3, -1), S(2, -7), S(0, 9), S(-64, -217), S(1, -15),
    },
    {
        S(7, 1), S(2, 1), S(7, 0), S(-10, 1), S(-1, -12), S(0, 0),
    },
};


// For a tapered evaluation
const int32_t game_phase_increment[6] = {
  0, 1, 1, 2, 4, 0
};

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

                // Phalanx pawns
                if (is_white ? (WHITE_LEFT_MASK[sq] & wp) : (BLACK_LEFT_MASK[sq] & bp)){
                    eval_array[is_white ? 0 : 1] += phalanx_pawns[is_white ? sq / 8 : 7 - sq / 8];
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

            // Defenders
            int32_t num_pawn_defends = is_white ? __builtin_popcountll(attacks_bb & wp.getBits()) : __builtin_popcountll(attacks_bb & bp.getBits());
            int32_t num_knight_defends = is_white ? __builtin_popcountll(attacks_bb & wn.getBits()) : __builtin_popcountll(attacks_bb & bn.getBits());
            int32_t num_bishop_defends = is_white ? __builtin_popcountll(attacks_bb & wb.getBits()) : __builtin_popcountll(attacks_bb & bb.getBits());
            int32_t num_rook_defends = is_white ? __builtin_popcountll(attacks_bb & wr.getBits()) : __builtin_popcountll(attacks_bb & br.getBits());
            int32_t num_queen_defends = is_white ? __builtin_popcountll(attacks_bb & wq.getBits()) : __builtin_popcountll(attacks_bb & bq.getBits());
            int32_t num_king_defends = is_white ? __builtin_popcountll(attacks_bb & wk.getBits()) : __builtin_popcountll(attacks_bb & bk.getBits());

            eval_array[is_white ? 0 : 1] += defenders[j][0] * num_pawn_defends;
            eval_array[is_white ? 0 : 1] += defenders[j][1] * num_knight_defends;
            eval_array[is_white ? 0 : 1] += defenders[j][2] * num_bishop_defends;
            eval_array[is_white ? 0 : 1] += defenders[j][3] * num_rook_defends;
            eval_array[is_white ? 0 : 1] += defenders[j][4] * num_queen_defends;
            eval_array[is_white ? 0 : 1] += defenders[j][5] * num_king_defends;

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
