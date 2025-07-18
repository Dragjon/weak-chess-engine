// error 0.0762893

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
// Rooks on open file bonus
// Like the rooks on semi open file bonus, this is also useful to encourage rook range
// and also improves endgame eval 

const int32_t PSQT[6][64] = {
    {
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
        S(42, 185), S(73, 197), S(67, 181), S(78, 145), S(78, 120), S(78, 131), S(47, 153), S(33, 160),
        S(82, 159), S(90, 182), S(91, 168), S(102, 150), S(114, 111), S(152, 114), S(130, 158), S(103, 136),
        S(79, 129), S(96, 138), S(93, 125), S(75, 117), S(92, 104), S(97, 106), S(85, 122), S(79, 100),
        S(74, 106), S(81, 121), S(89, 106), S(87, 113), S(81, 103), S(90, 97), S(72, 110), S(70, 87),
        S(89, 98), S(100, 108), S(89, 103), S(62, 114), S(81, 110), S(85, 102), S(94, 102), S(80, 83),
        S(81, 96), S(110, 100), S(99, 96), S(80, 108), S(74, 120), S(93, 100), S(102, 97), S(73, 77),
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
    },
    {
        S(4, 78), S(18, 143), S(60, 132), S(88, 124), S(123, 121), S(47, 105), S(42, 137), S(53, 55),
        S(98, 144), S(107, 129), S(85, 129), S(104, 129), S(100, 119), S(123, 110), S(110, 121), S(124, 125),
        S(109, 121), S(106, 126), S(154, 169), S(161, 172), S(175, 164), S(196, 148), S(113, 120), S(128, 113),
        S(120, 135), S(108, 137), S(154, 178), S(171, 182), S(153, 182), S(175, 178), S(111, 140), S(147, 125),
        S(108, 136), S(97, 133), S(138, 179), S(138, 182), S(145, 185), S(143, 170), S(116, 129), S(119, 127),
        S(92, 122), S(85, 128), S(122, 162), S(129, 175), S(140, 172), S(128, 156), S(102, 119), S(109, 121),
        S(103, 144), S(92, 128), S(76, 125), S(92, 125), S(91, 122), S(86, 121), S(105, 117), S(126, 152),
        S(81, 132), S(113, 135), S(81, 125), S(95, 123), S(99, 124), S(102, 115), S(114, 143), S(108, 119)
    },
    {
        S(110, 173), S(91, 178), S(96, 170), S(57, 180), S(73, 175), S(79, 168), S(114, 166), S(79, 168),
        S(104, 165), S(118, 165), S(115, 165), S(108, 164), S(108, 159), S(120, 160), S(99, 173), S(105, 164),
        S(125, 177), S(137, 167), S(133, 165), S(135, 159), S(130, 164), S(152, 168), S(142, 167), S(131, 178),
        S(115, 174), S(122, 171), S(127, 168), S(145, 176), S(135, 170), S(131, 171), S(122, 168), S(119, 169),
        S(122, 171), S(112, 173), S(125, 169), S(137, 172), S(137, 168), S(131, 164), S(123, 167), S(130, 160),
        S(125, 170), S(137, 166), S(133, 166), S(133, 166), S(135, 169), S(137, 162), S(140, 158), S(145, 160),
        S(144, 170), S(140, 155), S(142, 151), S(130, 161), S(137, 160), S(141, 156), S(155, 160), S(148, 154),
        S(136, 159), S(147, 165), S(136, 161), S(125, 166), S(130, 164), S(128, 172), S(137, 158), S(155, 141)
    },
    {
        S(134, 239), S(127, 243), S(131, 251), S(130, 249), S(133, 241), S(158, 235), S(138, 240), S(157, 234),
        S(130, 235), S(131, 242), S(146, 247), S(160, 238), S(145, 241), S(171, 225), S(167, 222), S(180, 217),
        S(124, 230), S(149, 226), S(142, 230), S(145, 225), S(170, 216), S(177, 208), S(199, 205), S(170, 207),
        S(123, 234), S(134, 228), S(136, 233), S(142, 230), S(138, 221), S(150, 212), S(144, 216), S(145, 213),
        S(110, 233), S(105, 235), S(115, 234), S(125, 233), S(121, 232), S(116, 226), S(124, 219), S(120, 218),
        S(105, 234), S(104, 231), S(111, 230), S(115, 233), S(118, 230), S(120, 220), S(135, 206), S(122, 211),
        S(108, 230), S(112, 230), S(122, 231), S(124, 229), S(126, 224), S(126, 219), S(138, 210), S(116, 218),
        S(125, 238), S(123, 233), S(125, 238), S(131, 231), S(131, 228), S(122, 232), S(130, 223), S(128, 225)
    },
    {
        S(323, 570), S(317, 575), S(336, 589), S(362, 575), S(346, 578), S(356, 576), S(400, 528), S(356, 564),
        S(347, 554), S(333, 565), S(330, 596), S(318, 614), S(310, 631), S(357, 575), S(361, 570), S(387, 574),
        S(356, 552), S(349, 558), S(351, 578), S(349, 584), S(358, 587), S(382, 574), S(391, 550), S(376, 565),
        S(339, 572), S(348, 570), S(343, 576), S(343, 589), S(342, 591), S(350, 583), S(357, 587), S(356, 578),
        S(349, 562), S(338, 577), S(342, 573), S(347, 584), S(343, 581), S(345, 572), S(350, 568), S(355, 573),
        S(344, 548), S(352, 556), S(346, 567), S(342, 563), S(345, 567), S(348, 562), S(360, 544), S(355, 553),
        S(348, 551), S(350, 547), S(358, 541), S(352, 553), S(353, 554), S(352, 525), S(359, 506), S(372, 499),
        S(344, 554), S(343, 546), S(347, 547), S(352, 558), S(350, 538), S(330, 537), S(341, 528), S(355, 522)
    },
    {
        S(79, -121), S(72, -69), S(104, -54), S(-20, -7), S(-39, 4), S(-29, 4), S(45, -14), S(174, -116),
        S(-57, -28), S(42, -7), S(20, 9), S(108, 2), S(20, 40), S(11, 48), S(31, 30), S(-15, 8),
        S(-77, -19), S(77, -6), S(36, 21), S(19, 36), S(20, 61), S(58, 45), S(24, 32), S(-37, 10),
        S(-41, -27), S(-4, 0), S(-8, 26), S(-46, 50), S(-72, 77), S(-51, 60), S(-67, 38), S(-126, 23),
        S(-58, -30), S(-14, -5), S(-24, 21), S(-52, 43), S(-72, 75), S(-40, 51), S(-59, 33), S(-131, 22),
        S(-26, -34), S(21, -14), S(-6, 8), S(-15, 20), S(-25, 53), S(-18, 40), S(-1, 20), S(-53, 10),
        S(30, -45), S(31, -20), S(22, -9), S(-1, 0), S(-18, 35), S(-5, 26), S(24, 8), S(4, -8),
        S(14, -80), S(44, -60), S(26, -40), S(-37, -21), S(-5, -6), S(-33, 6), S(16, -17), S(0, -44)
    },
};


const int32_t mobilities[5][28] = {

    {
        S(0, 0), S(0, 0), S(120, 202), S(140, 193), S(160, 223), S(0, 0), S(194, 235), S(0, 0), S(173, 207), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(118, 141), S(133, 137), S(137, 168), S(151, 178), S(156, 187), S(166, 201), S(171, 209), S(177, 217), S(179, 221), S(182, 227), S(181, 225), S(183, 223), S(200, 220), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(0, 0), S(190, 257), S(194, 275), S(198, 283), S(201, 289), S(201, 293), S(201, 298), S(204, 299), S(208, 302), S(209, 308), S(211, 311), S(212, 317), S(216, 321), S(215, 326), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(0, 0), S(0, 0), S(434, 350), S(424, 422), S(450, 475), S(453, 497), S(449, 557), S(452, 571), S(449, 592), S(452, 599), S(453, 610), S(454, 621), S(456, 624), S(459, 629), S(458, 637), S(458, 643), S(459, 648), S(458, 655), S(458, 659), S(458, 666), S(460, 666), S(462, 667), S(470, 662), S(468, 663), S(482, 658), S(516, 636), S(563, 610),
    },
    {
        S(0, 0), S(0, 0), S(0, 0), S(47, -8), S(52, -17), S(37, 0), S(27, 0), S(24, -5), S(20, -3), S(15, -3), S(14, -4), S(6, 1), S(5, -1), S(-2, 1), S(-11, 3), S(-22, 6), S(-35, 6), S(-51, 6), S(-63, 7), S(-78, 6), S(-78, 2), S(-83, 0), S(-90, -2), S(-97, -7), S(-113, -8), S(-97, -19), S(-110, -18), S(-86, -31),
    },
};


const int32_t bishop_pair = S(20, 58);


const int32_t passed_pawns[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(42, 185), S(73, 197), S(67, 181), S(78, 145), S(78, 120), S(78, 131), S(47, 153), S(33, 160),
    S(18, 145), S(33, 151), S(21, 115), S(7, 76), S(10, 87), S(12, 103), S(-17, 114), S(-40, 144),
    S(14, 80), S(18, 77), S(19, 53), S(15, 49), S(4, 47), S(10, 54), S(-1, 72), S(-11, 80),
    S(5, 44), S(0, 39), S(-11, 30), S(-5, 23), S(-12, 26), S(-7, 30), S(2, 41), S(1, 39),
    S(0, 8), S(-8, 17), S(-17, 13), S(-17, 7), S(-13, 6), S(-6, 6), S(8, 21), S(14, 6),
    S(-7, 7), S(-3, 12), S(-16, 13), S(-14, 5), S(1, -7), S(2, -1), S(20, 3), S(7, 6),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};

const int32_t inner_king_zone_attacks[4] = {
    S(10, -6), S(19, -3), S(20, -7), S(13, 7),
};


const int32_t outer_king_zone_attacks[4] = {
    S(0, 1), S(0, 0), S(4, -4), S(2, 1),
};


const int32_t doubled_pawn_penalty[8] = {
    S(-6, -40), S(-4, -26), S(-10, -23), S(-9, -10), S(-8, -20), S(-11, -22), S(-5, -25), S(-13, -35),
};


const int32_t pawn_storm[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(25, -61), S(-33, -53), S(-3, -92), S(-16, -49), S(-62, 49), S(-52, -10), S(-3, -8), S(-34, 1),
    S(-16, -33), S(-19, -42), S(10, -52), S(-1, -27), S(-11, 17), S(-30, 0), S(-42, -7), S(-44, 4),
    S(-29, -11), S(-29, -14), S(-20, -14), S(-2, -15), S(-8, 13), S(-17, 3), S(-33, 10), S(-45, 16),
    S(-31, -5), S(-24, -5), S(-20, -3), S(-7, -9), S(-5, 6), S(-21, 7), S(-27, 8), S(-42, 16),
    S(-46, 3), S(-45, 4), S(-24, 0), S(8, -5), S(-11, 1), S(-22, 3), S(-57, 17), S(-59, 23),
    S(-37, 8), S(-55, 10), S(-44, 9), S(-21, 3), S(-27, 4), S(-39, 8), S(-70, 27), S(-53, 37),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};

const int32_t isolated_pawns[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(49, -25), S(111, -105), S(52, -8), S(53, -6), S(17, 3), S(2, 26), S(-26, 21), S(0, 2),
    S(11, -17), S(13, -48), S(3, -25), S(5, -13), S(2, -17), S(-3, -8), S(22, -28), S(7, -25),
    S(7, -9), S(-1, -28), S(-6, -17), S(-6, -21), S(-5, -23), S(5, -20), S(19, -27), S(4, -7),
    S(-2, 1), S(-5, -12), S(-21, -6), S(-22, -19), S(-16, -19), S(-17, -6), S(-15, -9), S(-9, 2),
    S(-11, -2), S(-12, -15), S(-22, -10), S(-13, -17), S(-29, -15), S(-21, -7), S(-23, -15), S(-21, 0),
    S(-8, -3), S(-14, -8), S(-3, -10), S(-27, -5), S(-27, -10), S(-9, -5), S(-2, -16), S(-12, 3),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};

const int32_t threats[6][6] = {

    {
        S(3, -9), S(11, -44), S(26, -30), S(-19, -27), S(-10, -62), S(-88, 2),
    },
    {
        S(-10, 4), S(0, 0), S(20, 22), S(48, -7), S(23, -35), S(0, 0),
    },
    {
        S(-6, 5), S(15, 19), S(0, 0), S(31, 3), S(33, 68), S(0, 0),
    },
    {
        S(-17, 13), S(0, 15), S(9, 12), S(0, 0), S(45, 0), S(0, 0),
    },
    {
        S(-4, 5), S(0, 7), S(-3, 27), S(2, -3), S(0, 0), S(0, 0),
    },
    {
        S(37, 32), S(-40, 8), S(-18, 21), S(-15, -2), S(0, 0), S(0, 0),
    },
};


const int32_t rook_semi_open[2] = {
    S(22, 6), S(52, 8),
};


const int32_t rook_open_file[2] = {
    S(6, 130), S(51, 222),
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

    // Rook on open files
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
            }

            // Actual king attacks because we used queen attacks for virtual mobility above
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
    if (num_b_rooks_on_op_file == 1) eval_array[1] += rook_open_file[0];
    if (num_b_rooks_on_op_file == 2) eval_array[1] += rook_open_file[1];

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
