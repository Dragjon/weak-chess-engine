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
        S(62, 150), S(77, 144), S(68, 145), S(78, 119), S(72, 119), S(67, 127), S(38, 147), S(15, 153),
        S(69, 133), S(84, 140), S(106, 121), S(106, 126), S(118, 102), S(153, 111), S(144, 140), S(100, 124),
        S(55, 127), S(79, 126), S(78, 114), S(83, 97), S(101, 97), S(97, 107), S(99, 117), S(76, 104),
        S(45, 114), S(71, 121), S(74, 107), S(85, 101), S(88, 100), S(84, 105), S(83, 110), S(62, 96),
        S(46, 110), S(68, 118), S(71, 107), S(74, 109), S(84, 109), S(81, 108), S(97, 106), S(69, 93),
        S(48, 113), S(69, 119), S(63, 112), S(64, 116), S(75, 121), S(83, 112), S(101, 106), S(62, 93),
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
    },
    {
        S(16, 81), S(21, 142), S(54, 127), S(84, 117), S(120, 121), S(68, 98), S(51, 142), S(60, 59),
        S(116, 142), S(112, 124), S(100, 117), S(112, 119), S(90, 115), S(154, 99), S(118, 119), S(149, 126),
        S(112, 115), S(110, 117), S(153, 168), S(154, 173), S(185, 161), S(194, 151), S(122, 114), S(128, 110),
        S(112, 129), S(89, 133), S(139, 177), S(161, 178), S(129, 185), S(156, 180), S(82, 140), S(136, 122),
        S(101, 129), S(80, 124), S(126, 176), S(126, 176), S(138, 179), S(129, 168), S(95, 123), S(110, 119),
        S(83, 113), S(70, 116), S(114, 155), S(124, 166), S(134, 164), S(120, 148), S(87, 109), S(100, 113),
        S(98, 142), S(86, 118), S(64, 112), S(82, 111), S(80, 109), S(74, 107), S(93, 107), S(121, 148),
        S(77, 131), S(113, 130), S(77, 112), S(90, 113), S(95, 113), S(93, 104), S(112, 138), S(107, 119)
    },
    {
        S(108, 168), S(80, 177), S(83, 169), S(47, 179), S(64, 175), S(78, 166), S(99, 167), S(83, 164),
        S(112, 159), S(126, 157), S(117, 160), S(105, 161), S(119, 153), S(117, 156), S(117, 163), S(111, 159),
        S(124, 173), S(139, 161), S(135, 158), S(138, 152), S(128, 157), S(157, 161), S(141, 163), S(132, 176),
        S(120, 169), S(127, 165), S(129, 159), S(140, 170), S(136, 162), S(131, 165), S(127, 162), S(111, 170),
        S(122, 165), S(113, 167), S(117, 165), S(136, 163), S(132, 162), S(123, 160), S(118, 163), S(132, 156),
        S(125, 167), S(127, 165), S(127, 162), S(126, 160), S(127, 163), S(130, 159), S(131, 155), S(142, 159),
        S(137, 171), S(135, 154), S(136, 146), S(122, 157), S(130, 155), S(132, 151), S(144, 160), S(140, 154),
        S(130, 158), S(141, 166), S(132, 158), S(120, 161), S(125, 160), S(120, 169), S(132, 154), S(146, 142)
    },
    {
        S(188, 309), S(170, 317), S(166, 328), S(161, 325), S(171, 320), S(194, 314), S(191, 313), S(209, 306),
        S(89, 164), S(79, 176), S(94, 180), S(106, 173), S(86, 177), S(113, 164), S(108, 159), S(136, 147),
        S(160, 312), S(182, 311), S(175, 313), S(177, 311), S(200, 302), S(196, 299), S(234, 291), S(204, 289),
        S(154, 315), S(164, 310), S(163, 316), S(169, 311), S(164, 304), S(172, 298), S(173, 300), S(174, 294),
        S(139, 310), S(139, 310), S(147, 310), S(156, 306), S(151, 307), S(140, 305), S(154, 297), S(147, 296),
        S(134, 306), S(139, 302), S(146, 301), S(148, 302), S(149, 300), S(146, 294), S(166, 279), S(152, 284),
        S(136, 300), S(144, 299), S(158, 298), S(159, 297), S(160, 292), S(150, 290), S(163, 282), S(144, 290),
        S(152, 302), S(154, 300), S(162, 304), S(164, 300), S(163, 296), S(147, 300), S(161, 294), S(150, 292)
    },
    {
        S(333, 557), S(320, 571), S(331, 589), S(358, 573), S(344, 582), S(350, 579), S(395, 528), S(357, 556),
        S(348, 550), S(324, 573), S(322, 603), S(311, 620), S(300, 639), S(342, 591), S(342, 580), S(384, 572),
        S(353, 554), S(345, 563), S(344, 589), S(341, 595), S(347, 599), S(369, 583), S(380, 559), S(371, 561),
        S(335, 571), S(341, 575), S(334, 583), S(329, 599), S(329, 599), S(338, 589), S(347, 589), S(345, 579),
        S(344, 562), S(332, 577), S(334, 573), S(339, 585), S(334, 583), S(335, 573), S(340, 567), S(348, 568),
        S(339, 546), S(345, 552), S(340, 563), S(336, 558), S(338, 563), S(341, 555), S(351, 539), S(349, 547),
        S(348, 543), S(347, 540), S(354, 534), S(350, 543), S(349, 545), S(348, 512), S(356, 489), S(380, 479),
        S(344, 542), S(343, 537), S(347, 536), S(352, 545), S(349, 525), S(325, 525), S(346, 510), S(353, 514)
    },
    {
        S(29, -98), S(21, -46), S(60, -34), S(-63, 13), S(-29, -1), S(-12, -1), S(34, -14), S(135, -119),
        S(-90, -4), S(4, 26), S(-14, 37), S(84, 24), S(36, 41), S(30, 51), S(48, 38), S(-29, 8),
        S(-88, 6), S(54, 29), S(15, 56), S(-1, 69), S(38, 68), S(92, 59), S(40, 48), S(-24, 13),
        S(-54, -2), S(-23, 34), S(-28, 63), S(-56, 86), S(-51, 86), S(-32, 69), S(-50, 49), S(-115, 21),
        S(-70, -7), S(-28, 23), S(-36, 54), S(-60, 78), S(-54, 76), S(-29, 52), S(-56, 32), S(-131, 18),
        S(-38, -15), S(10, 9), S(-15, 34), S(-17, 46), S(-14, 45), S(-15, 33), S(-2, 11), S(-58, 0),
        S(19, -28), S(17, -1), S(14, 10), S(-11, 21), S(-11, 22), S(0, 13), S(26, -5), S(4, -22),
        S(-2, -62), S(42, -44), S(26, -21), S(-42, -4), S(6, -22), S(-26, -9), S(20, -33), S(-5, -58)
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
        S(0, 0), S(0, 0), S(113, 193), S(135, 188), S(158, 224), S(0, 0), S(195, 239), S(0, 0), S(166, 207), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),   S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(115, 134), S(127, 132), S(133, 163), S(146, 173), S(151, 182), S(162, 198), S(168, 204), S(173, 215), S(173, 219), S(176, 226), S(176, 223), S(175, 224), S(196, 218), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0,   0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(0, 0), S(185, 298), S(191, 312), S(197, 320), S(200, 325), S(200, 330), S(202, 336), S(207, 339), S(211, 342), S(217, 347), S(221, 351), S(223, 357), S(228, 360), S(227, 364), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0,   0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(0, 0), S(0, 0), S(420, 368), S(407, 440), S(432, 483), S(434, 503), S(431, 561), S(436, 571), S(435, 590), S(438, 595), S(440, 605), S(441, 616), S(443, 619), S(445, 623), S(445, 631), S(444, 636), S(445, 642), S(444  , 647), S(445, 650), S(446, 657), S(449, 654), S(450, 654), S(459, 647), S(459, 645), S(473, 638), S(508, 617), S(562, 590),
    },
    {
        S(0, 0), S(0, 0), S(0, 0), S(56, -17), S(63, -21), S(44, -1), S(33, 0), S(28, -3), S(21, 0), S(16, -1), S(15, -1), S(4, 4), S(1, 1), S(-7, 4), S(-17, 7), S(-30, 9), S(-45, 9), S(-61, 9), S(-75, 8), S(-91, 6), S(-93, 1), S(-100,   -3), S(-107, -8), S(-117, -14), S(-133, -18), S(-120, -33), S(-125, -37), S(-111, -50),
    },
};


// Bishop pair evaluation
const int32_t bishop_pair = S(20, 62);

// Passed pawn bonus
const int32_t passed_pawns[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(62, 150), S(77, 144), S(68, 145), S(78, 119), S(72, 119), S(67, 127), S(38, 147), S(15, 153),
    S(18, 131), S(30, 124), S(17, 101), S(5, 60), S(10, 78), S(4, 91), S(-22, 102), S(-45, 130),
    S(11, 65), S(8, 56), S(15, 40), S(12, 37), S(2, 36), S(10, 38), S(-18, 59), S(-19, 70),
    S(-4, 34), S(-12, 23), S(-20, 17), S(-12, 12), S(-20, 14), S(-13, 16), S(-20, 33), S(-12, 35),
    S(-10, -2), S(-19, 0), S(-27, 0), S(-25, -5), S(-24, -6), S(-18, -6), S(-20, 13), S(-4, 3),
    S(-17, -2), S(-11, -3), S(-20, -1), S(-26, -3), S(-14, -17), S(-11, -12), S(3, -5), S(-8, 2),
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

// Rooks on seventh
const int32_t pigs_on_seventh[8] = {
    S(78, 148), S(233, 322),
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

    // Rooks on seventh bonus
    chess::Bitboard masked_and_wr = chess::attacks::MASK_RANK[static_cast<int>(chess::Rank::RANK_7)] & wr;
    if (masked_and_wr){
        int32_t count = masked_and_wr.count();
        if (count < 3){
            score_array[0] += pigs_on_seventh[count - 1];
        }
    }

    chess::Bitboard masked_and_br = chess::attacks::MASK_RANK[static_cast<int>(chess::Rank::RANK_2)] & br;
    if (masked_and_br){
        int32_t count = masked_and_br.count();
        if (count < 3){
            score_array[1] += pigs_on_seventh[count - 1];
        }
    }

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
