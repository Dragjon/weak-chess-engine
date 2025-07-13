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
        S(37, 178), S(73, 190), S(65, 171), S(72, 135), S(75, 106), S(69, 118), S(34, 141), S(17, 155),
        S(82, 156), S(96, 171), S(92, 155), S(95, 141), S(112, 99), S(147, 104), S(131, 146), S(100, 132),
        S(80, 126), S(100, 129), S(97, 116), S(71, 108), S(97, 93), S(92, 99), S(87, 113), S(77, 97),
        S(73, 104), S(86, 114), S(91, 99), S(79, 108), S(85, 96), S(83, 93), S(74, 103), S(67, 85),
        S(88, 96), S(102, 103), S(89, 99), S(55, 110), S(81, 106), S(80, 98), S(93, 97), S(79, 82),
        S(82, 95), S(113, 97), S(99, 93), S(70, 110), S(72, 119), S(87, 99), S(101, 93), S(71, 77),
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
    },
    {
        S(12, 85), S(20, 143), S(55, 129), S(83, 120), S(121, 122), S(62, 100), S(51, 141), S(63, 63),
        S(110, 144), S(112, 127), S(101, 120), S(117, 122), S(99, 117), S(155, 102), S(125, 121), S(149, 124),
        S(113, 118), S(112, 120), S(157, 169), S(159, 174), S(190, 162), S(205, 149), S(127, 115), S(134, 110),
        S(116, 130), S(91, 134), S(144, 179), S(165, 180), S(136, 186), S(162, 182), S(89, 141), S(140, 122),
        S(105, 130), S(84, 127), S(130, 177), S(129, 178), S(141, 181), S(133, 169), S(101, 124), S(115, 121),
        S(87, 116), S(74, 120), S(117, 156), S(127, 168), S(138, 165), S(123, 151), S(92, 111), S(104, 115),
        S(101, 141), S(90, 119), S(68, 115), S(85, 113), S(83, 112), S(82, 108), S(102, 108), S(125, 147),
        S(86, 134), S(114, 131), S(82, 115), S(95, 114), S(100, 115), S(101, 105), S(114, 138), S(113, 121)
    },
    {
        S(107, 169), S(79, 178), S(83, 170), S(44, 182), S(62, 177), S(70, 169), S(106, 167), S(79, 166),
        S(105, 162), S(129, 158), S(118, 162), S(106, 163), S(121, 154), S(122, 159), S(115, 167), S(113, 159),
        S(125, 175), S(139, 163), S(138, 161), S(142, 154), S(131, 161), S(160, 164), S(143, 165), S(134, 177),
        S(118, 171), S(129, 166), S(131, 162), S(145, 173), S(139, 166), S(133, 168), S(130, 162), S(115, 171),
        S(124, 167), S(115, 168), S(120, 168), S(139, 167), S(136, 165), S(127, 161), S(121, 164), S(136, 156),
        S(127, 168), S(131, 166), S(130, 164), S(129, 162), S(131, 165), S(133, 160), S(135, 157), S(145, 159),
        S(141, 171), S(138, 154), S(140, 148), S(125, 158), S(132, 156), S(139, 152), S(152, 159), S(143, 154),
        S(132, 159), S(145, 165), S(135, 158), S(123, 162), S(129, 160), S(126, 169), S(134, 155), S(149, 142)
    },
    {
        S(178, 306), S(164, 313), S(164, 321), S(161, 319), S(172, 313), S(196, 308), S(183, 312), S(200, 304),
        S(165, 309), S(157, 320), S(174, 324), S(188, 317), S(170, 320), S(205, 303), S(197, 301), S(219, 289),
        S(156, 306), S(177, 305), S(171, 308), S(174, 305), S(200, 296), S(199, 291), S(228, 286), S(201, 284),
        S(148, 310), S(157, 306), S(161, 310), S(168, 306), S(163, 298), S(172, 292), S(169, 294), S(171, 290),
        S(134, 305), S(133, 306), S(144, 304), S(154, 301), S(151, 301), S(141, 298), S(153, 291), S(147, 290),
        S(130, 302), S(135, 297), S(144, 295), S(147, 297), S(149, 294), S(148, 287), S(168, 272), S(152, 278),
        S(132, 294), S(141, 294), S(156, 292), S(158, 291), S(159, 286), S(156, 281), S(170, 273), S(144, 282),
        S(149, 296), S(150, 296), S(159, 299), S(165, 293), S(164, 290), S(150, 292), S(162, 285), S(153, 283)
    },
    {
        S(329, 566), S(322, 578), S(336, 594), S(362, 580), S(352, 585), S(363, 581), S(407, 531), S(360, 564),
        S(349, 557), S(329, 577), S(327, 608), S(316, 627), S(308, 644), S(356, 591), S(359, 581), S(391, 578),
        S(356, 559), S(348, 569), S(349, 592), S(347, 599), S(356, 604), S(380, 586), S(388, 563), S(376, 569),
        S(339, 575), S(346, 577), S(341, 585), S(337, 603), S(337, 605), S(346, 594), S(354, 594), S(352, 584),
        S(349, 564), S(338, 580), S(339, 578), S(344, 591), S(340, 590), S(342, 578), S(348, 574), S(355, 574),
        S(346, 547), S(351, 558), S(346, 569), S(343, 564), S(345, 570), S(348, 563), S(359, 545), S(354, 554),
        S(353, 545), S(353, 544), S(361, 539), S(357, 548), S(356, 550), S(357, 520), S(363, 501), S(378, 491),
        S(348, 547), S(348, 542), S(354, 541), S(358, 551), S(357, 530), S(335, 531), S(348, 520), S(358, 516)
    },
    {
        S(76, -122), S(71, -68), S(105, -55), S(-25, -6), S(-39, 5), S(-13, 1), S(45, -13), S(164, -117),
        S(-57, -27), S(42, 2), S(17, 17), S(108, 7), S(21, 51), S(20, 58), S(38, 45), S(-10, 12),
        S(-63, -15), S(79, 7), S(33, 37), S(17, 51), S(17, 79), S(77, 68), S(41, 53), S(-22, 18),
        S(-35, -25), S(-2, 12), S(-13, 42), S(-50, 69), S(-77, 99), S(-45, 80), S(-50, 55), S(-109, 26),
        S(-60, -29), S(-18, 1), S(-27, 31), S(-55, 57), S(-78, 89), S(-46, 63), S(-62, 40), S(-127, 23),
        S(-28, -37), S(14, -12), S(-15, 12), S(-21, 25), S(-32, 58), S(-29, 44), S(-8, 20), S(-55, 7),
        S(30, -50), S(29, -24), S(19, -11), S(-1, -2), S(-23, 34), S(-8, 24), S(22, 4), S(4, -13),
        S(14, -85), S(44, -66), S(27, -45), S(-34, -26), S(-4, -10), S(-33, 1), S(17, -22), S(0, -49)
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
        S(0, 0), S(118, 138), S(132, 136), S(136, 166), S(149, 176), S(155, 185), S(166, 200), S(172, 206), S(176, 217), S(177, 220), S(180, 226), S(180, 223), S(179, 223), S(200, 218), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(0, 0), S(197, 307), S(203, 323), S(208, 331), S(212, 336), S(212, 341), S(213, 347), S(218, 350), S(223, 354), S(228, 359), S(233, 362), S(235, 368), S(241, 371), S(241, 374), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(0, 0), S(0, 0), S(436, 355), S(421, 438), S(446, 486), S(447, 506), S(444, 565), S(447, 575), S(446, 594), S(449, 600), S(450, 610), S(451, 622), S(453, 624), S(455, 629), S(455, 637), S(454, 643), S(455, 649), S(454, 654), S(455, 657), S(456, 664), S(459, 661), S(461, 662), S(469, 655), S(469, 654), S(484, 646), S(520, 623), S(571, 595),
    },
    {
        S(0, 0), S(0, 0), S(0, 0), S(45, -8), S(49, -11), S(32, 3), S(25, 4), S(23, -1), S(19, 0), S(15, 0), S(13, 0), S(6, 5), S(5, 2), S(-2, 5), S(-11, 7), S(-22, 8), S(-35, 8), S(-50, 7), S(-62, 7), S(-75, 4), S(-75, -1), S(-78, -6), S(-83, -11), S(-91, -17), S(-103, -22), S(-85, -37), S(-98, -41), S(-77, -54),
    },
};


// Bishop pair evaluation
const int32_t bishop_pair = S(20, 62);

// Passed pawn bonus
const int32_t passed_pawns[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(37, 178), S(73, 190), S(65, 171), S(72, 135), S(75, 106), S(69, 118), S(34, 141), S(17, 155),
    S(18, 142), S(32, 145), S(22, 111), S(5, 69), S(12, 80), S(12, 96), S(-19, 109), S(-48, 139),
    S(13, 78), S(19, 74), S(19, 51), S(16, 46), S(4, 43), S(14, 48), S(0, 68), S(-9, 76),
    S(3, 44), S(1, 37), S(-9, 28), S(-3, 22), S(-12, 25), S(-2, 26), S(7, 37), S(5, 37),
    S(0, 8), S(-6, 15), S(-15, 12), S(-16, 7), S(-11, 6), S(-2, 3), S(14, 18), S(18, 5),
    S(-7, 8), S(-1, 10), S(-13, 10), S(-13, 5), S(0, -7), S(6, -4), S(24, 1), S(12, 5),
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
const int32_t doubled_pawn_penalty[8] = {
    S(-8, -35), S(-4, -21), S(-8, -16), S(-9, -1), S(-7, -11), S(-8, -15), S(-5, -20), S(-15, -31),
};

// Pawn storm
const int32_t pawn_storm[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(20, -58), S(-42, -50), S(-6, -87), S(-8, -51), S(-60, 56), S(-44, -3), S(-13, 2), S(-49, 7),
    S(-25, -31), S(-24, -36), S(7, -48), S(7, -28), S(-9, 20), S(-27, 1), S(-46, -2), S(-56, 8),
    S(-34, -10), S(-32, -11), S(-24, -10), S(7, -16), S(-8, 15), S(-13, 5), S(-36, 13), S(-53, 19),
    S(-36, -5), S(-24, -5), S(-22, -1), S(3, -10), S(-3, 7), S(-15, 7), S(-28, 9), S(-50, 17),
    S(-49, 2), S(-45, 4), S(-23, 0), S(16, -7), S(-10, 0), S(-16, 1), S(-58, 17), S(-66, 24),
    S(-41, 8), S(-56, 10), S(-45, 9), S(-10, -1), S(-25, 3), S(-35, 7), S(-72, 28), S(-61, 38),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};


// Isolated pawns
const int32_t isolated_pawns[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(48, -26), S(110, -105), S(51, -6), S(54, -3), S(17, 10), S(9, 32), S(-39, 31), S(-4, 0),
    S(14, -21), S(11, -46), S(0, -22), S(4, -10), S(2, -14), S(-3, -3), S(13, -24), S(1, -25),
    S(10, -12), S(-4, -26), S(-10, -16), S(-9, -19), S(-8, -22), S(5, -18), S(9, -25), S(2, -9),
    S(0, -1), S(-10, -12), S(-24, -6), S(-24, -20), S(-16, -20), S(-16, -7), S(-25, -7), S(-11, 0),
    S(-10, -5), S(-15, -16), S(-27, -9), S(-14, -17), S(-30, -17), S(-21, -7), S(-34, -12), S(-22, -1),
    S(-6, -6), S(-16, -7), S(-9, -7), S(-28, -3), S(-27, -12), S(-10, -4), S(-11, -14), S(-12, 1),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};

// Double Isolated pawns
const int32_t doubled_isolated_pawns[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(-150, -95), S(-228, 4), S(-38, -92), S(-61, -154), S(64, 16), S(-30, -53), S(-57, -30), S(-15, -193),
    S(30, 56), S(11, -60), S(8, -29), S(-21, -17), S(27, -69), S(-13, -48), S(24, -12), S(43, -63),
    S(7, -3), S(6, -21), S(3, -20), S(4, -20), S(-6, -23), S(7, -21), S(25, -9), S(49, -17),
    S(3, 20), S(-6, 4), S(4, -8), S(8, -12), S(2, 2), S(-1, -10), S(18, -10), S(16, -2),
    S(-2, -1), S(5, -2), S(17, -13), S(7, -31), S(0, -3), S(3, -8), S(29, -4), S(6, -4),
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
                bool doubled = false;
                bool isolated = false;

                // Doubled pawns
                // Note that for pawns to be considered "doubled", they need not be directly in front
                // of another pawn
                uint64_t front_mask = is_white ? WHITE_AHEAD_MASK[sq] : BLACK_AHEAD_MASK[sq];
                uint64_t our_pawn_bb = is_white ? wp.getBits() : bp.getBits();
                if (front_mask & our_pawn_bb){
                    eval_array[is_white ? 0 : 1] += doubled_pawn_penalty[is_white ? 7 - sq % 8 : sq % 8];
                    doubled = true;
                }

                // Passed pawn
                if (is_white ? is_white_passed_pawn(sq, bp.getBits()): is_black_passed_pawn(sq, wp.getBits())){
                    eval_array[is_white ? 0 : 1] += passed_pawns[is_white ? sq ^ 56 : sq];
                    isolated = true;
                }

                // Pawn storm
                if (is_white ? (not_kingside_w_mask & (1ull << sq)) : (not_kingside_b_mask & (1ull << sq))){
                    eval_array[is_white ? 0 : 1] += pawn_storm[is_white ? sq ^ 56 : sq];
                }

                // Isolated pawn
                if ((LEFT_RIGHT_COLUMN_MASK[sq] & (is_white ? wp.getBits() : bp.getBits())) == 0ull){
                    eval_array[is_white ? 0 : 1] += isolated_pawns[is_white ? sq ^ 56 : sq];
                }

                // Doubled Isolated pawn
                if (doubled && isolated){
                    eval_array[is_white ? 0 : 1] += doubled_isolated_pawns[is_white ? sq ^ 56 : sq];
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
