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
        S(37, 178), S(71, 191), S(65, 172), S(70, 135), S(76, 106), S(69, 119), S(33, 141), S(17, 156),
        S(78, 155), S(94, 170), S(93, 153), S(93, 140), S(109, 98), S(145, 104), S(130, 143), S(103, 130),
        S(77, 122), S(100, 122), S(96, 111), S(71, 104), S(93, 94), S(98, 98), S(90, 110), S(85, 95),
        S(73, 104), S(85, 113), S(91, 97), S(77, 107), S(83, 95), S(82, 92), S(74, 101), S(69, 83),
        S(88, 97), S(100, 107), S(90, 100), S(58, 112), S(83, 107), S(81, 100), S(95, 100), S(81, 81),
        S(82, 94), S(111, 99), S(97, 94), S(67, 114), S(71, 119), S(85, 98), S(102, 93), S(72, 75),
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
    },
    {
        S(12, 85), S(19, 143), S(55, 128), S(83, 120), S(120, 122), S(63, 101), S(49, 142), S(63, 64),
        S(111, 144), S(112, 126), S(101, 119), S(118, 122), S(99, 117), S(155, 102), S(125, 121), S(149, 125),
        S(113, 118), S(113, 120), S(157, 169), S(159, 174), S(190, 162), S(204, 150), S(127, 116), S(134, 110),
        S(116, 130), S(92, 134), S(143, 179), S(165, 180), S(136, 186), S(162, 181), S(89, 141), S(141, 122),
        S(105, 129), S(85, 126), S(130, 177), S(128, 178), S(140, 180), S(132, 169), S(101, 124), S(115, 120),
        S(87, 115), S(75, 119), S(116, 157), S(126, 168), S(137, 165), S(123, 151), S(92, 111), S(104, 115),
        S(101, 140), S(89, 120), S(69, 115), S(86, 113), S(84, 111), S(83, 108), S(102, 108), S(125, 146),
        S(87, 134), S(114, 131), S(83, 115), S(96, 114), S(100, 115), S(102, 104), S(114, 138), S(113, 122)
    },
    {
        S(108, 169), S(79, 179), S(83, 171), S(44, 182), S(63, 177), S(71, 170), S(106, 168), S(79, 167),
        S(106, 163), S(130, 158), S(119, 162), S(107, 163), S(122, 154), S(122, 159), S(116, 167), S(112, 160),
        S(125, 175), S(139, 164), S(139, 161), S(142, 155), S(132, 161), S(161, 165), S(143, 165), S(134, 177),
        S(119, 171), S(129, 166), S(131, 163), S(144, 173), S(139, 167), S(133, 168), S(131, 162), S(115, 171),
        S(125, 167), S(116, 169), S(120, 168), S(139, 167), S(136, 165), S(127, 162), S(122, 164), S(136, 156),
        S(127, 168), S(131, 166), S(130, 164), S(129, 163), S(131, 166), S(133, 160), S(135, 157), S(144, 159),
        S(140, 171), S(138, 154), S(140, 149), S(126, 158), S(133, 157), S(138, 152), S(152, 159), S(143, 154),
        S(132, 159), S(145, 165), S(135, 158), S(124, 162), S(130, 160), S(126, 170), S(134, 155), S(149, 142)
    },
    {
        S(179, 307), S(166, 314), S(166, 322), S(162, 320), S(173, 314), S(197, 308), S(184, 313), S(199, 306),
        S(166, 309), S(159, 321), S(175, 325), S(189, 317), S(172, 320), S(206, 304), S(198, 301), S(221, 289),
        S(156, 307), S(178, 306), S(172, 308), S(176, 305), S(201, 297), S(200, 292), S(229, 287), S(202, 285),
        S(148, 310), S(157, 307), S(161, 311), S(169, 306), S(163, 299), S(172, 293), S(168, 296), S(171, 290),
        S(135, 305), S(135, 306), S(146, 305), S(155, 302), S(152, 301), S(142, 299), S(155, 291), S(149, 291),
        S(131, 302), S(136, 298), S(145, 297), S(147, 298), S(150, 295), S(149, 288), S(168, 273), S(152, 279),
        S(134, 295), S(142, 295), S(157, 293), S(158, 292), S(160, 287), S(156, 282), S(171, 273), S(145, 283),
        S(150, 297), S(151, 296), S(160, 300), S(166, 294), S(165, 291), S(151, 293), S(162, 286), S(154, 283)
    },
    {
        S(330, 567), S(322, 578), S(336, 595), S(363, 580), S(352, 585), S(363, 582), S(408, 531), S(361, 564),
        S(350, 558), S(330, 577), S(328, 608), S(316, 626), S(308, 645), S(356, 592), S(359, 581), S(391, 579),
        S(356, 559), S(348, 569), S(350, 592), S(348, 599), S(356, 605), S(381, 587), S(388, 565), S(376, 570),
        S(339, 576), S(346, 577), S(341, 586), S(336, 605), S(336, 606), S(346, 595), S(354, 595), S(352, 585),
        S(349, 565), S(337, 581), S(339, 579), S(345, 591), S(340, 590), S(342, 579), S(348, 573), S(355, 574),
        S(345, 547), S(351, 558), S(346, 570), S(343, 565), S(344, 571), S(348, 563), S(359, 546), S(354, 554),
        S(353, 545), S(353, 544), S(361, 540), S(356, 549), S(356, 550), S(356, 520), S(362, 501), S(377, 492),
        S(348, 547), S(348, 542), S(353, 541), S(358, 552), S(356, 530), S(335, 530), S(348, 520), S(358, 517)
    },
    {
        S(76, -122), S(71, -68), S(102, -54), S(-26, -6), S(-40, 6), S(-14, 2), S(43, -12), S(166, -118),
        S(-55, -28), S(41, 2), S(16, 17), S(107, 7), S(19, 51), S(20, 58), S(38, 44), S(-10, 12),
        S(-62, -15), S(80, 7), S(34, 37), S(18, 51), S(18, 79), S(77, 68), S(40, 54), S(-23, 18),
        S(-36, -25), S(0, 12), S(-13, 42), S(-49, 69), S(-77, 99), S(-45, 80), S(-50, 55), S(-110, 26),
        S(-60, -29), S(-17, 1), S(-26, 31), S(-54, 57), S(-79, 89), S(-46, 63), S(-63, 40), S(-128, 23),
        S(-27, -37), S(15, -13), S(-14, 12), S(-20, 25), S(-32, 58), S(-29, 44), S(-9, 19), S(-56, 7),
        S(31, -51), S(30, -24), S(20, -11), S(0, -2), S(-23, 34), S(-7, 24), S(22, 4), S(5, -14),
        S(14, -86), S(44, -65), S(27, -44), S(-34, -26), S(-4, -9), S(-33, 1), S(17, -22), S(0, -49)
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
        S(0, 0), S(0, 0), S(113, 194), S(140, 191), S(161, 226), S(0, 0), S(198, 240), S(0, 0), S(170, 208), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(120, 136), S(132, 136), S(137, 166), S(150, 176), S(155, 185), S(165, 200), S(172, 207), S(176, 217), S(177, 221), S(180, 226), S(180, 223), S(180, 223), S(201, 218), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(0, 0), S(196, 307), S(202, 323), S(208, 331), S(211, 336), S(212, 341), S(213, 347), S(218, 350), S(222, 354), S(227, 359), S(232, 362), S(234, 368), S(241, 371), S(241, 374), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(0, 0), S(0, 0), S(435, 356), S(422, 437), S(448, 485), S(447, 508), S(443, 568), S(448, 576), S(446, 594), S(449, 600), S(451, 611), S(452, 623), S(454, 625), S(456, 630), S(455, 638), S(455, 644), S(456, 650), S(455, 655), S(456, 658), S(457, 664), S(460, 662), S(462, 662), S(471, 655), S(470, 654), S(486, 646), S(522, 623), S(573, 595),
    },
    {
        S(0, 0), S(0, 0), S(0, 0), S(45, -7), S(50, -11), S(33, 4), S(27, 4), S(23, -1), S(19, 0), S(15, 0), S(14, 0), S(6, 5), S(5, 2), S(-2, 5), S(-12, 7), S(-22, 8), S(-36, 8), S(-52, 8), S(-63, 7), S(-77, 4), S(-76, -1), S(-79, -6), S(-84, -11), S(-92, -17), S(-104, -22), S(-86, -37), S(-100, -41), S(-77, -54),
    },
};


// Bishop pair evaluation
const int32_t bishop_pair = S(20, 62);


// Passed pawn bonus
const int32_t passed_pawns[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(37, 178), S(71, 191), S(65, 172), S(70, 135), S(76, 106), S(69, 119), S(33, 141), S(17, 156),
    S(18, 142), S(34, 146), S(21, 112), S(6, 70), S(16, 79), S(11, 98), S(-19, 111), S(-51, 141),
    S(21, 83), S(21, 80), S(22, 56), S(17, 50), S(10, 44), S(12, 52), S(1, 69), S(-13, 81),
    S(4, 45), S(2, 38), S(-10, 30), S(-2, 22), S(-10, 26), S(0, 29), S(6, 39), S(4, 42),
    S(0, 7), S(-5, 12), S(-18, 11), S(-21, 7), S(-14, 4), S(-2, 1), S(11, 15), S(16, 6),
    S(-5, 7), S(0, 9), S(-12, 9), S(-13, 2), S(0, -9), S(6, -4), S(23, 1), S(10, 7),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};


// King Zone Attacks
// Bonus for pieces (not pawns or kings) which attack squares near the king
const int32_t inner_king_zone_attacks[4] = {
    S(10, -5), S(17, -5), S(20, -6), S(12, 7),
};

// Attacking the ring 1 square away from the king square 
const int32_t outer_king_zone_attacks[4] = {
    S(0, 1), S(0, 0), S(3, -3), S(3, 0),
};


// Doubled pawn penalty
const int32_t doubled_pawn_penalty[8] = {
    S(-3, -36), S(-2, -23), S(-6, -21), S(-4, -6), S(-4, -15), S(-5, -20), S(-3, -22), S(-9, -32),
};


// Pawn storm
const int32_t pawn_storm[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(21, -58), S(-40, -52), S(-6, -87), S(-5, -52), S(-60, 55), S(-45, -2), S(-13, 2), S(-49, 4),
    S(-22, -31), S(-26, -36), S(6, -48), S(8, -28), S(-9, 19), S(-30, 1), S(-48, -2), S(-56, 8),
    S(-34, -11), S(-31, -11), S(-25, -10), S(6, -17), S(-9, 14), S(-15, 4), S(-36, 13), S(-53, 19),
    S(-36, -5), S(-25, -4), S(-22, -1), S(4, -10), S(-4, 7), S(-16, 8), S(-29, 10), S(-50, 18),
    S(-49, 2), S(-44, 3), S(-22, 0), S(19, -8), S(-8, 1), S(-15, 2), S(-57, 17), S(-65, 23),
    S(-42, 8), S(-56, 10), S(-46, 10), S(-8, -1), S(-25, 2), S(-35, 7), S(-72, 28), S(-60, 38),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};


// Isolated pawns
const int32_t isolated_pawns[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(48, -25), S(111, -107), S(52, -9), S(52, -3), S(15, 9), S(7, 28), S(-30, 29), S(0, -2),
    S(15, -19), S(14, -47), S(1, -24), S(4, -10), S(2, -15), S(-1, -7), S(24, -24), S(7, -27),
    S(0, -11), S(-14, -24), S(-14, -18), S(-9, -22), S(-19, -26), S(-10, -22), S(-1, -24), S(-4, -14),
    S(-1, -2), S(-12, -12), S(-21, -9), S(-23, -18), S(-17, -21), S(-23, -11), S(-20, -10), S(-13, -4),
    S(-12, -4), S(-13, -16), S(-24, -10), S(-12, -18), S(-26, -16), S(-22, -9), S(-25, -13), S(-22, -2),
    S(-8, -3), S(-14, -7), S(-2, -9), S(-27, -8), S(-28, -11), S(-8, -4), S(-4, -14), S(-13, 2),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};


// Backward pawns
const int32_t backward_pawns[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(17, 12), S(21, 11), S(13, 9), S(2, 14), S(17, 14), S(28, 16), S(31, 5), S(13, 20),
    S(0, -2), S(5, -3), S(-8, 0), S(-1, -10), S(-2, -2), S(13, 0), S(9, 0), S(9, 3),
    S(0, -7), S(-8, -17), S(-9, -10), S(-13, -7), S(-17, -9), S(-2, -8), S(-12, -11), S(-2, -3),
    S(5, -6), S(0, -6), S(1, -8), S(-4, -16), S(-13, 0), S(-2, 3), S(10, 2), S(13, -2),
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

                // Backward pawns - pawns that cannot safely advance due to their front being guarded
                // by an enemy pawn, and all the pawns in the adjacent column have advanced after it
                if (is_white ? (((WHITE_ADJ_FILES_DOWNWARD_MASK[sq] & wp.getBits()) == 0ull) && (WHITE_DIAGONAL_UP_THEN_FORWARD_MASK[sq] & bp.getBits())) : (((BLACK_ADJ_FILES_UPWARD_MASK[sq] & bp.getBits()) == 0ull) && (BLACK_DIAGONAL_DOWN_THEN_FORWARD_MASK[sq] & wp.getBits()))){
                    eval_array[is_white ? 0 : 1] += backward_pawns[is_white ? sq ^ 56 : sq];
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
