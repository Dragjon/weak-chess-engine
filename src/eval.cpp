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
// Rook on open file
// Bonus is given for one and 2 rooks on open files to encourage rook actiity and control
// more squares.

const int32_t PSQT[6][64] = {
    {
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
        S(45, 179), S(75, 191), S(70, 177), S(80, 142), S(79, 119), S(80, 129), S(49, 150), S(33, 159),
        S(85, 160), S(93, 180), S(92, 166), S(103, 146), S(114, 108), S(156, 113), S(132, 157), S(104, 136),
        S(79, 128), S(93, 135), S(92, 124), S(76, 114), S(92, 101), S(96, 104), S(82, 119), S(77, 99),
        S(73, 103), S(78, 118), S(85, 103), S(83, 112), S(77, 101), S(86, 95), S(69, 106), S(69, 85),
        S(89, 98), S(98, 107), S(89, 104), S(64, 113), S(82, 110), S(87, 102), S(94, 102), S(81, 83),
        S(84, 99), S(112, 104), S(102, 100), S(83, 108), S(75, 123), S(96, 103), S(105, 101), S(76, 80),
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
    },
    {
        S(4, 78), S(17, 144), S(61, 133), S(90, 125), S(124, 122), S(50, 106), S(42, 138), S(54, 55),
        S(98, 144), S(108, 129), S(86, 129), S(105, 129), S(101, 119), S(123, 110), S(111, 121), S(125, 125),
        S(111, 122), S(107, 126), S(154, 169), S(162, 171), S(174, 164), S(196, 149), S(115, 121), S(129, 114),
        S(121, 135), S(109, 137), S(154, 178), S(171, 182), S(154, 182), S(175, 179), S(112, 140), S(148, 125),
        S(109, 136), S(98, 132), S(137, 179), S(136, 181), S(144, 185), S(142, 170), S(117, 129), S(119, 127),
        S(93, 122), S(85, 128), S(121, 162), S(128, 175), S(139, 172), S(127, 156), S(103, 119), S(110, 122),
        S(102, 145), S(92, 129), S(77, 126), S(92, 125), S(91, 123), S(86, 121), S(105, 118), S(127, 152),
        S(81, 131), S(113, 135), S(82, 125), S(95, 125), S(99, 126), S(103, 115), S(114, 143), S(108, 119)
    },
    {
        S(109, 173), S(89, 178), S(95, 169), S(56, 180), S(72, 175), S(79, 168), S(114, 166), S(78, 169),
        S(104, 165), S(116, 165), S(114, 165), S(106, 164), S(108, 159), S(119, 160), S(98, 173), S(104, 164),
        S(124, 177), S(137, 168), S(131, 166), S(134, 160), S(128, 164), S(151, 168), S(141, 168), S(130, 178),
        S(115, 174), S(121, 172), S(125, 168), S(142, 176), S(133, 171), S(129, 171), S(121, 168), S(118, 169),
        S(121, 171), S(110, 173), S(123, 169), S(135, 172), S(134, 168), S(129, 164), S(121, 167), S(129, 161),
        S(123, 170), S(135, 167), S(131, 166), S(130, 166), S(133, 169), S(135, 162), S(138, 159), S(143, 159),
        S(142, 170), S(139, 155), S(140, 152), S(128, 161), S(135, 160), S(139, 156), S(154, 159), S(146, 153),
        S(135, 159), S(145, 166), S(135, 160), S(123, 166), S(128, 164), S(126, 172), S(134, 158), S(154, 141)
    },
    {
        S(136, 240), S(128, 244), S(133, 252), S(132, 250), S(135, 242), S(160, 237), S(140, 241), S(159, 236),
        S(131, 237), S(133, 243), S(147, 249), S(162, 240), S(147, 242), S(172, 227), S(168, 224), S(181, 218),
        S(126, 232), S(151, 228), S(144, 231), S(147, 226), S(171, 218), S(179, 210), S(201, 206), S(171, 209),
        S(124, 235), S(135, 229), S(137, 235), S(144, 231), S(139, 223), S(151, 214), S(146, 217), S(147, 215),
        S(111, 234), S(107, 236), S(116, 235), S(125, 234), S(122, 233), S(117, 227), S(126, 220), S(122, 219),
        S(106, 236), S(106, 232), S(112, 232), S(116, 235), S(119, 231), S(121, 221), S(136, 207), S(124, 213),
        S(109, 231), S(114, 231), S(123, 232), S(125, 230), S(127, 225), S(127, 220), S(139, 211), S(117, 219),
        S(127, 239), S(124, 234), S(126, 239), S(132, 232), S(132, 229), S(123, 234), S(131, 224), S(129, 226)
    },
    {
        S(322, 573), S(316, 578), S(335, 592), S(362, 577), S(346, 580), S(357, 578), S(402, 529), S(356, 565),
        S(346, 556), S(332, 567), S(330, 598), S(317, 615), S(309, 634), S(356, 578), S(360, 573), S(387, 576),
        S(356, 554), S(349, 560), S(351, 581), S(349, 587), S(357, 590), S(382, 577), S(391, 553), S(376, 568),
        S(339, 573), S(347, 572), S(343, 579), S(342, 592), S(341, 594), S(349, 586), S(357, 589), S(355, 581),
        S(349, 565), S(337, 579), S(341, 575), S(345, 586), S(343, 583), S(344, 575), S(350, 571), S(355, 575),
        S(344, 550), S(351, 558), S(345, 570), S(341, 566), S(344, 570), S(347, 565), S(359, 546), S(355, 555),
        S(347, 554), S(350, 550), S(357, 543), S(352, 554), S(352, 555), S(351, 528), S(359, 509), S(371, 502),
        S(343, 556), S(343, 548), S(347, 549), S(351, 560), S(349, 540), S(329, 539), S(340, 529), S(354, 525)
    },
    {
        S(79, -121), S(75, -70), S(103, -53), S(-15, -9), S(-38, 3), S(-27, 4), S(43, -13), S(163, -115),
        S(-58, -28), S(42, -7), S(22, 9), S(109, 1), S(21, 40), S(15, 47), S(32, 30), S(-17, 8),
        S(-76, -19), S(77, -5), S(37, 21), S(19, 36), S(21, 61), S(57, 46), S(26, 33), S(-36, 10),
        S(-43, -27), S(-3, 0), S(-6, 26), S(-46, 50), S(-72, 77), S(-51, 60), S(-66, 38), S(-124, 22),
        S(-56, -30), S(-12, -6), S(-23, 20), S(-51, 42), S(-71, 74), S(-39, 51), S(-57, 33), S(-130, 21),
        S(-26, -34), S(22, -14), S(-6, 7), S(-15, 20), S(-25, 52), S(-18, 40), S(-2, 19), S(-51, 9),
        S(30, -46), S(31, -21), S(23, -9), S(-1, 0), S(-18, 35), S(-4, 26), S(25, 7), S(4, -9),
        S(14, -81), S(44, -60), S(26, -40), S(-37, -21), S(-5, -5), S(-33, 6), S(16, -17), S(0, -45)
    },
};


const int32_t mobilities[5][28] = {

    {
        S(0, 0), S(0, 0), S(120, 203), S(140, 193), S(160, 223), S(0, 0), S(194, 236), S(0, 0), S(174, 208), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(120, 144), S(134, 139), S(139, 169), S(153, 179), S(157, 188), S(168, 202), S(173, 209), S(179, 217), S(181, 222), S(185, 227), S(184, 225), S(186, 223), S(203, 221), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(0, 0), S(188, 258), S(192, 275), S(197, 283), S(200, 289), S(200, 293), S(200, 298), S(203, 299), S(206, 303), S(208, 308), S(209, 311), S(210, 317), S(214, 321), S(213, 326), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(0, 0), S(0, 0), S(434, 346), S(424, 428), S(451, 474), S(453, 497), S(450, 558), S(452, 573), S(449, 594), S(452, 602), S(453, 613), S(454, 625), S(456, 627), S(458, 632), S(458, 640), S(458, 646), S(459, 651), S(458, 657), S(459, 661), S(459, 669), S(461, 669), S(462, 670), S(470, 665), S(468, 667), S(482, 661), S(515, 641), S(564, 614),
    },
    {
        S(0, 0), S(0, 0), S(0, 0), S(48, -6), S(52, -15), S(37, 2), S(27, 0), S(24, -5), S(19, -3), S(15, -3), S(14, -3), S(6, 1), S(5, -1), S(-3, 1), S(-12, 4), S(-22, 6), S(-35, 6), S(-51, 6), S(-63, 7), S(-78, 6), S(-78, 2), S(-83, 0), S(-89, -2), S(-96, -7), S(-112, -8), S(-96, -19), S(-109, -18), S(-88, -31),
    },
};


const int32_t bishop_pair = S(20, 58);


const int32_t passed_pawns[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(45, 179), S(75, 191), S(70, 177), S(80, 142), S(79, 119), S(80, 129), S(49, 150), S(33, 159),
    S(16, 143), S(29, 144), S(21, 107), S(8, 73), S(12, 84), S(10, 97), S(-20, 106), S(-42, 140),
    S(13, 79), S(19, 76), S(19, 52), S(13, 48), S(2, 46), S(10, 53), S(0, 70), S(-11, 78),
    S(5, 45), S(1, 39), S(-9, 30), S(-2, 23), S(-11, 26), S(-5, 31), S(2, 43), S(0, 40),
    S(0, 9), S(-8, 18), S(-17, 13), S(-17, 8), S(-13, 7), S(-6, 6), S(7, 22), S(14, 7),
    S(-7, 7), S(-3, 11), S(-17, 12), S(-14, 6), S(1, -7), S(1, -1), S(19, 2), S(7, 7),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};

const int32_t inner_king_zone_attacks[4] = {
    S(10, -6), S(18, -3), S(21, -7), S(13, 7),
};


const int32_t outer_king_zone_attacks[4] = {
    S(0, 1), S(0, 0), S(4, -4), S(2, 1),
};


const int32_t doubled_pawn_penalty[8] = {
    S(-6, -39), S(-4, -26), S(-11, -24), S(-10, -10), S(-9, -19), S(-12, -22), S(-5, -25), S(-14, -35),
};


const int32_t pawn_storm[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(25, -60), S(-33, -53), S(-4, -93), S(-16, -48), S(-61, 49), S(-48, -11), S(-4, -9), S(-32, 0),
    S(-18, -33), S(-19, -41), S(11, -52), S(-1, -27), S(-10, 17), S(-30, 0), S(-44, -4), S(-44, 5),
    S(-30, -10), S(-29, -13), S(-21, -13), S(-3, -14), S(-9, 13), S(-20, 5), S(-34, 11), S(-47, 17),
    S(-32, -5), S(-25, -4), S(-22, -2), S(-8, -8), S(-5, 6), S(-23, 8), S(-28, 8), S(-43, 16),
    S(-45, 3), S(-44, 4), S(-23, 0), S(7, -5), S(-11, 2), S(-22, 3), S(-57, 17), S(-58, 23),
    S(-37, 7), S(-56, 10), S(-45, 9), S(-23, 4), S(-27, 4), S(-39, 8), S(-71, 27), S(-53, 37),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};

const int32_t isolated_pawns[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(44, -14), S(106, -92), S(46, 0), S(48, -1), S(16, 5), S(0, 28), S(-29, 27), S(-2, 5),
    S(11, -16), S(12, -41), S(1, -16), S(3, -5), S(1, -11), S(-5, -3), S(22, -21), S(7, -22),
    S(10, -7), S(1, -25), S(-5, -15), S(-6, -18), S(-5, -20), S(8, -17), S(22, -24), S(7, -6),
    S(0, 2), S(-1, -11), S(-16, -6), S(-18, -19), S(-12, -18), S(-12, -5), S(-10, -7), S(-7, 3),
    S(-12, -2), S(-12, -15), S(-23, -11), S(-15, -17), S(-30, -16), S(-21, -8), S(-22, -15), S(-21, 0),
    S(-10, -6), S(-16, -10), S(-5, -13), S(-29, -6), S(-28, -12), S(-10, -7), S(-4, -19), S(-13, 1),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};

const int32_t threats[6][6] = {

    {
        S(3, -10), S(12, -44), S(26, -30), S(-19, -27), S(-10, -62), S(-91, 2),
    },
    {
        S(-11, 4), S(0, 0), S(20, 22), S(48, -6), S(22, -35), S(0, 0),
    },
    {
        S(-6, 5), S(15, 19), S(0, 0), S(31, 3), S(33, 68), S(0, 0),
    },
    {
        S(-17, 12), S(0, 15), S(9, 12), S(0, 0), S(45, 1), S(0, 0),
    },
    {
        S(-4, 5), S(0, 7), S(-3, 27), S(2, -3), S(0, 0), S(0, 0),
    },
    {
        S(36, 31), S(-40, 8), S(-18, 21), S(-15, -2), S(0, 0), S(0, 0),
    },
};


const int32_t rook_semi_open[2] = {
    S(23, 6), S(53, 9),
};


const int32_t phalanx_pawns[8] = {
    S(0, 0), S(-2, -7), S(-1, -3), S(18, 11), S(47, 39), S(124, 170), S(-132, 424), S(0, 0),
};


const int32_t rook_open_file[2] = {
    S(6, 131), S(51, 223),
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

    // Rooks on open files
    int32_t num_w_rooks_on_op_file = 0;
    int32_t num_b_rooks_on_op_file = 0;

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

                        // Rook on open file
                        if ((is_white ? (WHITE_AHEAD_MASK[sq] & wp & bp) : (BLACK_AHEAD_MASK[sq] & wp & bp)) == 0ull){
                            if (is_white) num_w_rooks_on_op_file++;
                            else num_b_rooks_on_op_file++;
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

    // Rooks on open files
    if (num_w_rooks_on_op_file == 1) eval_array[0] += rook_open_file[0];
    if (num_w_rooks_on_op_file == 2) eval_array[0] += rook_open_file[1];
    if (num_b_rooks_on_op_file == 1) eval_array[0] += rook_open_file[0];
    if (num_b_rooks_on_op_file == 2) eval_array[0] += rook_open_file[1];


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
