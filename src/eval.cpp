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
        S(62, 150), S(77, 144), S(68, 145), S(78, 119), S(72, 119), S(67, 127), S(38, 146), S(15, 153),
        S(68, 133), S(84, 140), S(106, 120), S(106, 126), S(118, 102), S(153, 111), S(144, 140), S(100, 124),
        S(55, 127), S(79, 126), S(78, 114), S(83, 97), S(101, 97), S(97, 107), S(99, 117), S(76, 104),
        S(45, 114), S(71, 121), S(74, 107), S(85, 101), S(88, 100), S(84, 105), S(83, 110), S(62, 96),
        S(46, 110), S(68, 118), S(71, 107), S(74, 109), S(84, 109), S(81, 108), S(97, 106), S(69, 93),
        S(48, 113), S(69, 119), S(63, 112), S(64, 116), S(75, 121), S(83, 112), S(101, 106), S(62, 93),
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
    },
    {
        S(16, 81), S(21, 142), S(54, 126), S(84, 116), S(120, 120), S(68, 97), S(51, 142), S(60, 59),
        S(116, 142), S(111, 124), S(100, 116), S(111, 119), S(90, 114), S(154, 98), S(118, 119), S(149, 126),
        S(112, 115), S(110, 116), S(153, 167), S(154, 173), S(185, 160), S(194, 151), S(122, 113), S(128, 110),
        S(112, 128), S(88, 132), S(139, 177), S(161, 178), S(129, 185), S(156, 180), S(82, 139), S(136, 121),
        S(101, 128), S(80, 123), S(126, 176), S(126, 176), S(138, 179), S(129, 168), S(95, 123), S(110, 118),
        S(83, 113), S(70, 115), S(114, 155), S(124, 166), S(134, 164), S(120, 148), S(87, 108), S(100, 113),
        S(98, 142), S(86, 117), S(64, 112), S(82, 110), S(79, 109), S(74, 107), S(93, 106), S(121, 148),
        S(77, 131), S(113, 130), S(77, 111), S(90, 113), S(95, 112), S(93, 104), S(112, 138), S(107, 119)
    },
    {
        S(107, 167), S(80, 177), S(83, 169), S(47, 179), S(64, 174), S(78, 166), S(99, 167), S(83, 164),
        S(112, 159), S(126, 156), S(117, 159), S(105, 161), S(119, 152), S(117, 156), S(117, 163), S(111, 159),
        S(124, 173), S(139, 161), S(135, 158), S(138, 152), S(128, 157), S(157, 161), S(141, 162), S(132, 176),
        S(120, 168), S(127, 165), S(129, 159), S(140, 170), S(136, 162), S(131, 165), S(127, 161), S(111, 170),
        S(122, 165), S(113, 167), S(117, 165), S(136, 163), S(132, 162), S(123, 160), S(118, 163), S(132, 156),
        S(125, 167), S(127, 165), S(127, 162), S(126, 160), S(127, 163), S(130, 159), S(131, 155), S(142, 158),
        S(137, 171), S(135, 153), S(136, 146), S(122, 157), S(130, 154), S(132, 151), S(144, 160), S(140, 153),
        S(130, 158), S(141, 166), S(132, 157), S(120, 161), S(125, 160), S(120, 168), S(132, 154), S(146, 142)
    },
    {
        S(184, 301), S(165, 309), S(162, 320), S(157, 317), S(167, 312), S(189, 306), S(187, 305), S(205, 298),
        S(167, 305), S(157, 317), S(172, 322), S(186, 314), S(165, 319), S(193, 306), S(190, 300), S(216, 288),
        S(156, 304), S(179, 303), S(172, 304), S(174, 303), S(197, 294), S(192, 291), S(231, 283), S(201, 281),
        S(150, 307), S(161, 302), S(160, 308), S(166, 303), S(161, 296), S(168, 289), S(170, 291), S(171, 286),
        S(136, 302), S(136, 302), S(144, 302), S(153, 298), S(148, 298), S(137, 297), S(150, 289), S(144, 288),
        S(131, 298), S(135, 294), S(143, 292), S(145, 294), S(145, 291), S(143, 285), S(163, 271), S(149, 275),
        S(133, 291), S(141, 291), S(155, 289), S(155, 289), S(156, 284), S(147, 281), S(160, 274), S(141, 281),
        S(149, 293), S(151, 292), S(159, 295), S(161, 292), S(160, 288), S(143, 291), S(158, 285), S(147, 283)
    },
    {
        S(332, 557), S(320, 570), S(332, 588), S(358, 572), S(343, 582), S(350, 579), S(395, 528), S(357, 556),
        S(349, 550), S(324, 573), S(322, 602), S(311, 619), S(300, 639), S(342, 591), S(342, 580), S(384, 572),
        S(353, 554), S(345, 563), S(344, 588), S(341, 594), S(347, 599), S(369, 582), S(380, 558), S(371, 561),
        S(335, 571), S(341, 574), S(334, 582), S(329, 598), S(329, 599), S(338, 588), S(347, 588), S(345, 578),
        S(344, 562), S(332, 577), S(334, 572), S(339, 584), S(334, 582), S(335, 572), S(340, 567), S(348, 567),
        S(339, 546), S(345, 552), S(340, 562), S(336, 558), S(338, 563), S(341, 555), S(351, 538), S(349, 546),
        S(348, 543), S(347, 539), S(354, 534), S(350, 542), S(349, 544), S(348, 511), S(356, 488), S(380, 479),
        S(344, 541), S(343, 537), S(347, 535), S(352, 544), S(349, 524), S(325, 525), S(346, 510), S(353, 513)
    },
    {
        S(33, -98), S(27, -46), S(65, -35), S(-60, 13), S(-29, -1), S(-10, -1), S(36, -15), S(137, -119),
        S(-89, -4), S(5, 26), S(-14, 37), S(84, 25), S(37, 41), S(31, 51), S(48, 38), S(-28, 8),
        S(-87, 6), S(55, 29), S(16, 56), S(0, 69), S(38, 68), S(93, 59), S(41, 48), S(-23, 13),
        S(-53, -2), S(-23, 35), S(-27, 63), S(-56, 86), S(-51, 87), S(-31, 69), S(-49, 49), S(-114, 21),
        S(-69, -7), S(-27, 23), S(-36, 54), S(-61, 79), S(-54, 76), S(-30, 52), S(-56, 32), S(-130, 18),
        S(-38, -15), S(10, 9), S(-15, 34), S(-18, 46), S(-14, 45), S(-16, 33), S(-2, 11), S(-58, 0),
        S(19, -28), S(16, -1), S(14, 10), S(-11, 21), S(-12, 22), S(0, 13), S(26, -5), S(4, -22),
        S(-2, -62), S(41, -43), S(26, -21), S(-42, -4), S(6, -22), S(-27, -9), S(20, -33), S(-5, -58)
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
        S(0, 0), S(0, 0), S(112, 193), S(135, 188), S(158, 225), S(0, 0), S(195, 240), S(0, 0), S(166, 207), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(115, 133), S(127, 132), S(133, 163), S(146, 173), S(151, 182), S(162, 198), S(168, 204), S(173, 215), S(173, 219), S(176, 226), S(176, 223), S(175, 224), S(195, 218), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(0, 0), S(189, 307), S(195, 321), S(200, 329), S(203, 334), S(203, 339), S(205, 345), S(210, 347), S(215, 351), S(220, 356), S(224, 359), S(226, 365), S(231, 369), S(230, 372), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(0, 0), S(0, 0), S(421, 368), S(408, 440), S(432, 483), S(434, 503), S(432, 561), S(437, 571), S(435, 590), S(439, 595), S(440, 605), S(441, 616), S(443, 618), S(445, 623), S(445, 630), S(444, 636), S(445, 642), S(444, 647), S(445, 649), S(446, 656), S(449, 654), S(451, 653), S(459, 646), S(460, 644), S(474, 637), S(510, 616), S(564, 587),
    },
    {
        S(0, 0), S(0, 0), S(0, 0), S(57, -19), S(63, -21), S(44, -2), S(33, 0), S(27, -3), S(21, 0), S(16, -1), S(15, -1), S(3, 4), S(1, 1), S(-7, 5), S(-17, 7), S(-30, 9), S(-45, 10), S(-61, 9), S(-75, 8), S(-91, 6), S(-93, 1), S(-100, -3), S(-106, -8), S(-116, -14), S(-131, -19), S(-118, -33), S(-123, -37), S(-108, -51),
    },
};


// Bishop pair evaluation
const int32_t bishop_pair = S(20, 62);

// Passed pawn bonus
const int32_t passed_pawns[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(62, 150), S(77, 144), S(68, 145), S(78, 119), S(72, 119), S(67, 127), S(38, 146), S(15, 153),
    S(18, 131), S(30, 125), S(17, 101), S(5, 61), S(10, 78), S(4, 91), S(-22, 102), S(-45, 130),
    S(11, 65), S(8, 56), S(15, 40), S(12, 37), S(2, 36), S(10, 38), S(-18, 59), S(-19, 70),
    S(-4, 34), S(-12, 23), S(-20, 17), S(-12, 12), S(-20, 14), S(-13, 16), S(-20, 33), S(-12, 35),
    S(-10, -2), S(-19, 0), S(-27, 0), S(-25, -5), S(-24, -6), S(-18, -6), S(-20, 13), S(-4, 3),
    S(-16, -2), S(-11, -3), S(-20, -1), S(-26, -3), S(-14, -17), S(-11, -11), S(3, -5), S(-8, 2),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};


// King Zone Attacks
// Bonus for pieces (not pawns or kings) which attack squares near the king
const int32_t inner_king_zone_attacks[4] = {
    S(12, -6), S(18, -5), S(24, -7), S(14, 7),
};
// Attacking the ring 1 square away from the king square 
const int32_t outer_king_zone_attacks[4] = {
    S(0, 1), S(0, 0), S(3, -3), S(2, 1),
};


// Doubled pawn penalty
const int32_t doubled_pawn_penalty[8] = {
    S(-13, -48), S(-15, -34), S(-16, -36), S(-15, -18), S(-15, -29), S(-17, -34), S(-16, -31), S(-22, -42),
};


// For a tapered evaluation
const int32_t game_phase_increment[6] = {0, 1, 1, 2, 4, 0};

// This is our HCE evaluation function. 
int32_t evaluate(const chess::Board& board) {

    int32_t score_array[2] = {0,0};
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
            score_array[is_white ? 0 : 1] += PSQT[j][is_white ? sq ^ 56 : sq];

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
                score_array[is_white ? 0 : 1] += mobilities[j-1][attacks];
                
                // Non king non pawn pieces
                if (j < 5){
                    score_array[is_white ? 0 : 1] += inner_king_zone_attacks[j-1]  * count((is_white ? black_king_inner_sq_mask : white_king_inner_sq_mask) & attacks_bb); 
                    score_array[is_white ? 0 : 1] += outer_king_zone_attacks[j-1]  * count((is_white ? black_king_2_sq_mask : white_king_2_sq_mask) & attacks_bb); 
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
                    score_array[is_white ? 0 : 1] += doubled_pawn_penalty[is_white ? 7 - sq % 8 : sq % 8];

                // Passed pawn
                if (is_white ? is_white_passed_pawn(sq, bp.getBits()): is_black_passed_pawn(sq, wp.getBits())){
                    score_array[is_white ? 0 : 1] += passed_pawns[is_white ? sq ^ 56 : sq];
                }

            }

        }

    }

    // Rooks on open files
    /*
    if (num_w_rooks_on_op_file == 1) score_array[0] += rook_open_file[0];
    if (num_w_rooks_on_op_file == 2) score_array[0] += rook_open_file[1];
    if (num_b_rooks_on_op_file == 1) score_array[1] += rook_open_file[0];
    if (num_b_rooks_on_op_file == 2) score_array[1] += rook_open_file[1];
    */

    // Bishop Pair
    if (wb.count() == 2) score_array[0] += bishop_pair;
    if (bb.count() == 2) score_array[1] += bishop_pair;

    int32_t stm = board.sideToMove() == Color::WHITE ? 0 : 1;
    int32_t score = score_array[stm] - score_array[stm^1];
    int32_t mg_score = (int32_t)unpack_mg(score);
    int32_t eg_score = (int32_t)unpack_eg(score);
    int32_t mg_phase = phase;
    if (mg_phase > 24) mg_phase = 24;
    int32_t eg_phase = 24 - mg_phase; 

    // Evaluation tapering, that is, interpolating mg and eg values depending on how many pieces
    // there are on the board. See here for more information: https://www.chessprogramming.org/Tapered_Eval
    return tempo.current + ((mg_score * mg_phase + eg_score * eg_phase) / 24);
}
