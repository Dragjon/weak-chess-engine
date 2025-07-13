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

// Piece Square Tables (which includes the material value of each piece)
// Each piece type has its own piece square table, whiched it used
// to evaluate its positioning on the board
const int32_t PSQT[6][64] = {
    {
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
        S(38, 177), S(73, 191), S(65, 172), S(71, 135), S(75, 107), S(69, 118), S(33, 141), S(16, 156),
        S(83, 155), S(93, 171), S(91, 156), S(96, 141), S(113, 99), S(147, 105), S(133, 146), S(100, 132),
        S(80, 125), S(98, 128), S(97, 115), S(71, 108), S(97, 92), S(93, 99), S(90, 112), S(78, 97),
        S(74, 104), S(84, 113), S(91, 99), S(79, 108), S(86, 95), S(83, 93), S(78, 102), S(68, 85),
        S(89, 96), S(99, 102), S(89, 99), S(55, 110), S(81, 106), S(80, 98), S(97, 96), S(80, 81),
        S(82, 95), S(111, 96), S(98, 94), S(70, 110), S(73, 119), S(87, 100), S(105, 92), S(71, 77),
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
    },
    {
        S(12, 85), S(19, 143), S(56, 129), S(83, 120), S(121, 122), S(63, 100), S(50, 141), S(63, 63),
        S(110, 144), S(112, 127), S(101, 120), S(118, 122), S(99, 117), S(155, 102), S(125, 121), S(150, 124),
        S(113, 118), S(112, 120), S(157, 169), S(159, 174), S(190, 162), S(205, 149), S(127, 115), S(134, 110),
        S(116, 130), S(91, 134), S(144, 178), S(165, 180), S(136, 186), S(162, 182), S(89, 141), S(141, 123),
        S(105, 130), S(84, 127), S(130, 177), S(129, 178), S(141, 180), S(133, 169), S(101, 124), S(115, 121),
        S(87, 116), S(74, 120), S(117, 156), S(127, 168), S(138, 165), S(123, 151), S(92, 110), S(104, 115),
        S(101, 141), S(90, 119), S(68, 115), S(85, 113), S(83, 111), S(82, 108), S(102, 108), S(125, 146),
        S(86, 133), S(114, 131), S(82, 115), S(96, 114), S(100, 115), S(101, 105), S(114, 138), S(113, 122)
    },
    {
        S(108, 169), S(79, 178), S(82, 170), S(44, 182), S(62, 177), S(70, 169), S(105, 167), S(79, 166),
        S(105, 162), S(129, 158), S(118, 162), S(106, 163), S(121, 154), S(122, 159), S(115, 167), S(114, 159),
        S(124, 174), S(139, 163), S(138, 161), S(141, 154), S(131, 161), S(160, 164), S(143, 165), S(134, 177),
        S(118, 170), S(129, 166), S(131, 162), S(145, 173), S(139, 166), S(133, 168), S(130, 162), S(115, 171),
        S(124, 167), S(115, 168), S(120, 168), S(139, 167), S(136, 165), S(127, 161), S(121, 164), S(136, 156),
        S(127, 168), S(131, 166), S(130, 164), S(129, 162), S(130, 165), S(133, 160), S(135, 157), S(145, 159),
        S(141, 171), S(138, 154), S(140, 149), S(125, 158), S(132, 156), S(139, 152), S(152, 159), S(143, 154),
        S(132, 159), S(144, 165), S(135, 158), S(123, 162), S(129, 160), S(126, 169), S(134, 155), S(149, 142)
    },
    {
        S(177, 306), S(164, 313), S(164, 321), S(160, 319), S(172, 313), S(196, 307), S(183, 312), S(200, 304),
        S(165, 309), S(157, 320), S(174, 324), S(188, 317), S(170, 320), S(205, 303), S(197, 301), S(220, 289),
        S(156, 306), S(177, 305), S(171, 308), S(174, 305), S(199, 296), S(199, 291), S(228, 285), S(201, 284),
        S(148, 310), S(157, 306), S(161, 310), S(168, 306), S(163, 298), S(172, 292), S(169, 294), S(171, 289),
        S(134, 305), S(133, 306), S(144, 304), S(154, 301), S(151, 301), S(141, 298), S(153, 291), S(147, 290),
        S(130, 302), S(135, 297), S(144, 295), S(147, 297), S(149, 294), S(147, 287), S(168, 272), S(152, 278),
        S(132, 294), S(141, 294), S(156, 292), S(157, 291), S(159, 286), S(156, 281), S(170, 272), S(144, 282),
        S(149, 296), S(150, 296), S(159, 299), S(165, 293), S(164, 290), S(150, 292), S(162, 285), S(153, 283)
    },
    {
        S(330, 566), S(321, 578), S(336, 594), S(362, 580), S(352, 585), S(363, 581), S(407, 531), S(360, 563),
        S(349, 557), S(329, 577), S(327, 608), S(316, 626), S(308, 644), S(356, 591), S(359, 581), S(392, 578),
        S(356, 558), S(348, 569), S(349, 591), S(347, 598), S(356, 604), S(380, 586), S(389, 563), S(376, 569),
        S(339, 574), S(346, 577), S(341, 585), S(337, 603), S(337, 605), S(347, 594), S(354, 594), S(352, 584),
        S(349, 564), S(338, 580), S(339, 578), S(344, 591), S(340, 590), S(342, 579), S(348, 573), S(355, 574),
        S(346, 547), S(351, 558), S(346, 569), S(343, 564), S(345, 570), S(348, 562), S(359, 545), S(354, 554),
        S(353, 545), S(353, 544), S(361, 539), S(357, 548), S(356, 549), S(357, 520), S(363, 501), S(377, 491),
        S(348, 547), S(348, 542), S(354, 541), S(358, 551), S(357, 530), S(335, 531), S(348, 520), S(358, 516)
    },
    {
        S(75, -121), S(72, -68), S(105, -55), S(-25, -6), S(-40, 5), S(-13, 1), S(45, -13), S(164, -118),
        S(-56, -27), S(42, 3), S(17, 17), S(107, 7), S(21, 51), S(19, 59), S(38, 45), S(-9, 12),
        S(-63, -15), S(80, 7), S(34, 37), S(18, 51), S(17, 79), S(76, 68), S(41, 53), S(-22, 18),
        S(-34, -25), S(-2, 12), S(-13, 42), S(-50, 69), S(-77, 99), S(-45, 80), S(-51, 55), S(-109, 26),
        S(-60, -29), S(-18, 1), S(-27, 31), S(-55, 57), S(-78, 89), S(-46, 63), S(-62, 40), S(-128, 23),
        S(-27, -37), S(15, -12), S(-15, 12), S(-21, 25), S(-32, 58), S(-30, 44), S(-8, 19), S(-55, 7),
        S(31, -50), S(29, -24), S(20, -11), S(0, -2), S(-23, 34), S(-8, 24), S(22, 4), S(4, -13),
        S(14, -85), S(44, -66), S(28, -45), S(-33, -26), S(-4, -10), S(-33, 1), S(17, -22), S(0, -49)
    },
};


// Mobility Eval
// Each piece type can attack at most 28 squares in any given turn. In
// general, the more squares a piece attacks the better. But for example
// pieces like queens may get negative mobility in the opening to prevent
// early queen moves. The 4th index (index 5) of the mobilities array is 
// dedicated to king virtual mobility. That is putting a queen on a king sq
// and getting the mobility of the queen from there. This helps to position
// the king away from open rays
const int32_t mobilities[5][28] = {

    {
        S(0, 0), S(0, 0), S(113, 194), S(140, 191), S(161, 225), S(0, 0), S(198, 240), S(0, 0), S(170, 208), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(118, 138), S(132, 136), S(136, 166), S(149, 176), S(155, 185), S(166, 200), S(172, 206), S(177, 217), S(177, 220), S(180, 226), S(180, 223), S(179, 223), S(200, 218), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(0, 0), S(197, 307), S(203, 323), S(208, 331), S(212, 336), S(212, 341), S(213, 347), S(218, 350), S(223, 354), S(228, 359), S(233, 362), S(235, 368), S(241, 371), S(241, 374), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(0, 0), S(0, 0), S(438, 358), S(421, 439), S(446, 485), S(447, 505), S(444, 564), S(447, 575), S(446, 594), S(449, 599), S(450, 610), S(451, 622), S(453, 624), S(455, 629), S(455, 637), S(454, 643), S(455, 649), S(454, 654), S(455, 657), S(456, 664), S(459, 661), S(461, 661), S(469, 655), S(469, 654), S(484, 646), S(520, 623), S(571, 595),
    },
    {
        S(0, 0), S(0, 0), S(0, 0), S(45, -8), S(49, -11), S(32, 3), S(25, 4), S(23, -1), S(19, 0), S(14, 0), S(13, 0), S(6, 5), S(5, 2), S(-2, 5), S(-11, 7), S(-22, 8), S(-35, 8), S(-50, 7), S(-62, 7), S(-76, 4), S(-75, -1), S(-78, -6), S(-83, -11), S(-91, -17), S(-103, -22), S(-85, -37), S(-99, -41), S(-77, -54),
    },
};


// Bishop pair evaluation
const int32_t bishop_pair = S(20, 62);

// Passed pawn bonus
const int32_t passed_pawns[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(38, 177), S(73, 191), S(65, 172), S(71, 135), S(75, 107), S(69, 118), S(33, 141), S(16, 156),
    S(19, 142), S(32, 146), S(22, 111), S(5, 69), S(12, 80), S(12, 96), S(-21, 110), S(-50, 140),
    S(14, 79), S(19, 75), S(19, 51), S(16, 46), S(4, 43), S(13, 49), S(-2, 68), S(-11, 76),
    S(4, 44), S(2, 38), S(-11, 29), S(-3, 22), S(-12, 25), S(-3, 26), S(4, 38), S(2, 38),
    S(0, 8), S(-5, 16), S(-16, 12), S(-17, 7), S(-12, 6), S(-2, 3), S(11, 19), S(15, 6),
    S(-5, 8), S(0, 11), S(-14, 11), S(-13, 5), S(0, -7), S(5, -4), S(22, 2), S(8, 6),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};

// King Zone Attacks
// Bonus for pieces (not pawns or kings) which attack squares near the king
const int32_t inner_king_zone_attacks[4] = {
    S(9, -5), S(17, -4), S(20, -6), S(12, 7),
};

// Attacking the ring 1 square away from the king square 
const int32_t outer_king_zone_attacks[4] = {
    S(0, 1), S(0, 0), S(3, -3), S(3, 0),
};

// Doubled pawn penalty
const int32_t doubled_pawn_penalty[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(-162, -141), S(-142, -41), S(21, -103), S(-27, -159), S(127, 46), S(16, -37), S(61, -99), S(-27, -210),
    S(9, 15), S(-2, -77), S(-9, -24), S(-20, -18), S(37, -70), S(21, -43), S(51, -61), S(24, -101),
    S(-2, -35), S(10, -35), S(0, -29), S(-6, -12), S(-20, -12), S(-8, -21), S(8, -23), S(34, -50),
    S(-6, -13), S(-2, -17), S(-17, -14), S(-1, -12), S(-7, -5), S(-16, -14), S(-2, -23), S(0, -34),
    S(-13, -34), S(0, -20), S(0, -23), S(-1, -20), S(-19, -7), S(-6, -21), S(-8, -20), S(-9, -38),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};

// Pawn storm
const int32_t pawn_storm[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(20, -58), S(-44, -50), S(-5, -87), S(-8, -51), S(-60, 55), S(-43, -4), S(-13, 2), S(-48, 6),
    S(-26, -31), S(-24, -37), S(7, -48), S(7, -28), S(-9, 19), S(-27, 1), S(-46, -2), S(-55, 8),
    S(-34, -10), S(-32, -11), S(-24, -10), S(6, -16), S(-8, 15), S(-12, 5), S(-36, 13), S(-53, 19),
    S(-36, -5), S(-24, -5), S(-21, -1), S(3, -10), S(-3, 7), S(-15, 7), S(-29, 9), S(-49, 17),
    S(-49, 2), S(-45, 4), S(-23, 0), S(16, -7), S(-10, 0), S(-16, 1), S(-58, 17), S(-66, 24),
    S(-41, 8), S(-56, 10), S(-45, 9), S(-10, -1), S(-25, 2), S(-35, 7), S(-72, 28), S(-61, 38),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};

// Isolated pawns
const int32_t isolated_pawns[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(47, -25), S(111, -106), S(52, -8), S(53, -4), S(17, 8), S(7, 29), S(-37, 31), S(1, 0),
    S(12, -21), S(13, -47), S(1, -25), S(3, -11), S(1, -15), S(-3, -7), S(18, -25), S(5, -26),
    S(7, -12), S(-4, -26), S(-9, -18), S(-9, -21), S(-8, -22), S(6, -22), S(14, -26), S(5, -10),
    S(-2, -2), S(-9, -12), S(-22, -8), S(-24, -21), S(-16, -21), S(-15, -10), S(-18, -9), S(-8, 0),
    S(-12, -6), S(-14, -16), S(-24, -11), S(-13, -18), S(-29, -17), S(-20, -9), S(-28, -14), S(-19, -2),
    S(-8, -6), S(-15, -7), S(-6, -9), S(-28, -6), S(-24, -14), S(-9, -5), S(-3, -15), S(-9, 1),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
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

            // Mobilities for knight - queen, and king virtual mobility
            // King Zone
            if (j > 0){
                int32_t attacks = 0;
                uint64_t attacks_bb = 0ull;
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
                if (front_mask & our_pawn_bb){
                    eval_array[is_white ? 0 : 1] += doubled_pawn_penalty[is_white ? sq ^ 56 : sq];
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
    return tempo.current + ((mg_score * mg_phase + eg_score * eg_phase) / 24);
}
