// error 0.0762852

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
// Minor piece blocking pawn malus
// In the opening, it is usually bad to put pieces directly in front of pawns. The 0th
// index of this feature array is the malus given to knights blocking pawns while the
// 1st index is the malus given to bishop blocking pawns

const int32_t PSQT[6][64] = {
    {
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
        S(45, 184), S(74, 197), S(69, 180), S(79, 144), S(79, 119), S(79, 130), S(48, 152), S(34, 159),
        S(84, 159), S(92, 181), S(93, 167), S(104, 149), S(115, 112), S(155, 113), S(131, 157), S(104, 135),
        S(81, 129), S(98, 137), S(94, 125), S(77, 117), S(93, 103), S(99, 106), S(86, 121), S(80, 99),
        S(76, 105), S(83, 120), S(90, 105), S(87, 113), S(82, 103), S(91, 96), S(73, 110), S(71, 87),
        S(90, 97), S(101, 107), S(91, 103), S(62, 114), S(81, 110), S(88, 101), S(95, 102), S(82, 83),
        S(83, 96), S(112, 100), S(103, 95), S(82, 108), S(75, 120), S(96, 100), S(103, 97), S(74, 77),
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
    },
    {
        S(14, 81), S(21, 142), S(57, 128), S(86, 119), S(121, 116), S(47, 100), S(45, 136), S(63, 59),
        S(100, 143), S(104, 124), S(83, 124), S(102, 124), S(97, 115), S(121, 106), S(107, 116), S(126, 124),
        S(107, 117), S(104, 121), S(153, 166), S(160, 169), S(174, 161), S(195, 146), S(111, 116), S(125, 109),
        S(118, 131), S(106, 133), S(152, 175), S(170, 179), S(151, 180), S(174, 176), S(108, 137), S(145, 121),
        S(108, 132), S(95, 129), S(138, 176), S(136, 179), S(145, 182), S(142, 168), S(115, 126), S(117, 123),
        S(94, 119), S(87, 124), S(122, 160), S(128, 171), S(139, 169), S(132, 155), S(105, 117), S(111, 118),
        S(104, 144), S(89, 124), S(74, 121), S(89, 121), S(88, 119), S(84, 117), S(102, 113), S(128, 152),
        S(87, 138), S(114, 135), S(79, 120), S(92, 119), S(96, 120), S(99, 111), S(115, 143), S(115, 125)
    },
    {
        S(111, 170), S(90, 176), S(95, 167), S(57, 178), S(73, 172), S(80, 166), S(114, 163), S(79, 165),
        S(103, 163), S(117, 163), S(114, 163), S(107, 162), S(108, 157), S(119, 158), S(97, 171), S(103, 162),
        S(124, 175), S(136, 165), S(132, 163), S(134, 157), S(129, 162), S(152, 166), S(141, 165), S(129, 176),
        S(115, 173), S(121, 170), S(126, 166), S(144, 175), S(134, 169), S(130, 170), S(121, 166), S(118, 168),
        S(122, 169), S(111, 172), S(125, 168), S(137, 170), S(136, 166), S(130, 163), S(122, 166), S(129, 160),
        S(126, 170), S(138, 166), S(133, 165), S(131, 164), S(134, 167), S(138, 162), S(142, 159), S(146, 160),
        S(142, 168), S(138, 153), S(141, 149), S(128, 159), S(135, 158), S(139, 154), S(153, 158), S(146, 152),
        S(135, 157), S(146, 164), S(135, 159), S(124, 164), S(129, 162), S(126, 171), S(135, 156), S(154, 139)
    },
    {
        S(158, 303), S(150, 308), S(154, 316), S(153, 314), S(156, 306), S(180, 301), S(161, 305), S(181, 299),
        S(155, 300), S(157, 306), S(170, 312), S(186, 302), S(171, 305), S(196, 290), S(192, 287), S(205, 281),
        S(150, 295), S(174, 291), S(167, 294), S(170, 289), S(195, 281), S(203, 272), S(223, 270), S(196, 272),
        S(148, 298), S(159, 292), S(162, 297), S(168, 294), S(163, 285), S(176, 276), S(170, 280), S(171, 278),
        S(137, 297), S(131, 299), S(142, 298), S(151, 296), S(148, 295), S(143, 289), S(150, 283), S(147, 282),
        S(132, 298), S(131, 294), S(139, 293), S(142, 296), S(145, 292), S(148, 282), S(162, 269), S(150, 274),
        S(135, 293), S(139, 293), S(149, 294), S(151, 292), S(154, 287), S(153, 282), S(165, 273), S(143, 282),
        S(153, 301), S(150, 296), S(153, 300), S(159, 293), S(159, 290), S(150, 295), S(157, 286), S(155, 287)
    },
    {
        S(328, 563), S(320, 569), S(338, 584), S(365, 570), S(348, 572), S(359, 571), S(404, 523), S(360, 557),
        S(349, 549), S(335, 561), S(331, 592), S(319, 610), S(312, 627), S(359, 571), S(363, 566), S(389, 570),
        S(358, 548), S(351, 554), S(353, 574), S(351, 580), S(359, 584), S(384, 570), S(393, 546), S(379, 561),
        S(341, 567), S(350, 566), S(345, 572), S(344, 586), S(344, 588), S(352, 579), S(359, 583), S(358, 574),
        S(351, 558), S(340, 572), S(344, 569), S(349, 580), S(345, 578), S(347, 568), S(352, 564), S(357, 569),
        S(347, 544), S(354, 551), S(348, 563), S(344, 559), S(347, 563), S(350, 558), S(362, 540), S(357, 548),
        S(351, 546), S(353, 543), S(360, 537), S(355, 548), S(355, 549), S(354, 521), S(362, 501), S(374, 495),
        S(345, 550), S(346, 541), S(350, 542), S(354, 554), S(352, 534), S(332, 533), S(343, 523), S(356, 519)
    },
    {
        S(74, -119), S(70, -68), S(104, -53), S(-21, -6), S(-39, 4), S(-31, 5), S(43, -14), S(166, -114),
        S(-58, -28), S(42, -7), S(20, 9), S(108, 2), S(21, 40), S(11, 47), S(31, 29), S(-17, 8),
        S(-77, -19), S(77, -6), S(37, 20), S(20, 36), S(22, 60), S(60, 45), S(25, 32), S(-37, 10),
        S(-42, -27), S(-3, 0), S(-8, 26), S(-45, 49), S(-71, 76), S(-50, 60), S(-66, 38), S(-126, 23),
        S(-58, -29), S(-14, -5), S(-24, 21), S(-51, 42), S(-71, 74), S(-39, 50), S(-58, 33), S(-131, 21),
        S(-26, -33), S(20, -13), S(-6, 8), S(-16, 20), S(-26, 52), S(-18, 40), S(-1, 19), S(-52, 10),
        S(30, -45), S(30, -20), S(22, -9), S(-1, 0), S(-18, 35), S(-5, 26), S(24, 8), S(4, -7),
        S(14, -79), S(43, -59), S(26, -40), S(-38, -21), S(-5, -6), S(-33, 6), S(16, -17), S(0, -44)
    },
};


const int32_t mobilities[5][28] = {

    {
        S(0, 0), S(0, 0), S(115, 195), S(140, 192), S(164, 226), S(0, 0), S(198, 238), S(0, 0), S(176, 209), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(120, 142), S(136, 138), S(140, 169), S(154, 179), S(159, 187), S(169, 202), S(174, 209), S(180, 217), S(182, 222), S(185, 228), S(184, 225), S(187, 223), S(204, 220), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(0, 0), S(197, 304), S(201, 321), S(206, 330), S(208, 336), S(209, 340), S(209, 345), S(212, 347), S(215, 350), S(217, 355), S(219, 359), S(220, 365), S(223, 369), S(222, 374), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(0, 0), S(0, 0), S(461, 240), S(435, 417), S(459, 473), S(461, 495), S(458, 556), S(460, 570), S(458, 591), S(460, 599), S(461, 610), S(463, 622), S(465, 624), S(467, 629), S(467, 637), S(466, 643), S(468, 648), S(466, 655), S(467, 659), S(467, 666), S(469, 666), S(470, 667), S(479, 662), S(477, 663), S(491, 657), S(525, 635), S(573, 609),
    },
    {
        S(0, 0), S(0, 0), S(0, 0), S(47, -8), S(52, -18), S(36, 0), S(27, 0), S(23, -5), S(19, -3), S(15, -3), S(14, -4), S(6, 1), S(5, -1), S(-2, 1), S(-11, 3), S(-21, 5), S(-35, 6), S(-51, 6), S(-63, 6), S(-78, 6), S(-78, 2), S(-83, 0), S(-90, -2), S(-98, -7), S(-113, -7), S(-97, -19), S(-111, -17), S(-87, -31),
    },
};


const int32_t bishop_pair = S(19, 59);


const int32_t passed_pawns[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(45, 184), S(74, 197), S(69, 180), S(79, 144), S(79, 119), S(79, 130), S(48, 152), S(34, 159),
    S(18, 144), S(33, 150), S(19, 115), S(7, 75), S(10, 86), S(11, 103), S(-17, 114), S(-40, 143),
    S(15, 79), S(18, 77), S(19, 53), S(15, 48), S(3, 47), S(10, 54), S(-1, 72), S(-10, 79),
    S(5, 44), S(0, 39), S(-10, 30), S(-4, 23), S(-12, 26), S(-6, 30), S(3, 41), S(1, 39),
    S(0, 8), S(-8, 17), S(-17, 13), S(-17, 7), S(-13, 6), S(-6, 6), S(8, 21), S(15, 6),
    S(-7, 7), S(-3, 12), S(-17, 13), S(-15, 5), S(0, -7), S(1, -1), S(20, 3), S(7, 6),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};

const int32_t inner_king_zone_attacks[4] = {
    S(10, -6), S(19, -4), S(20, -7), S(13, 7),
};


const int32_t outer_king_zone_attacks[4] = {
    S(0, 1), S(0, 0), S(4, -4), S(2, 1),
};


const int32_t doubled_pawn_penalty[8] = {
    S(-7, -39), S(-5, -26), S(-12, -23), S(-10, -10), S(-8, -20), S(-13, -21), S(-6, -25), S(-14, -35),
};


const int32_t pawn_storm[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(21, -59), S(-35, -53), S(-5, -91), S(-17, -49), S(-61, 49), S(-52, -10), S(-3, -9), S(-35, 2),
    S(-18, -33), S(-21, -41), S(10, -52), S(-1, -27), S(-11, 17), S(-30, 0), S(-42, -8), S(-44, 4),
    S(-29, -11), S(-30, -13), S(-21, -13), S(-3, -15), S(-8, 13), S(-17, 3), S(-33, 10), S(-45, 16),
    S(-32, -5), S(-24, -5), S(-20, -3), S(-7, -9), S(-4, 6), S(-21, 7), S(-27, 8), S(-41, 16),
    S(-46, 3), S(-45, 4), S(-24, 0), S(8, -5), S(-11, 1), S(-22, 3), S(-57, 17), S(-59, 23),
    S(-37, 8), S(-55, 10), S(-45, 9), S(-21, 2), S(-27, 4), S(-40, 9), S(-71, 27), S(-53, 37),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};

const int32_t isolated_pawns[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(49, -26), S(111, -106), S(53, -8), S(54, -7), S(18, 3), S(4, 26), S(-26, 21), S(0, 2),
    S(11, -17), S(14, -48), S(3, -25), S(5, -12), S(3, -17), S(-2, -8), S(22, -28), S(7, -25),
    S(7, -9), S(-1, -27), S(-7, -17), S(-7, -21), S(-5, -23), S(5, -20), S(19, -27), S(4, -7),
    S(-2, 1), S(-5, -12), S(-21, -6), S(-22, -19), S(-15, -19), S(-17, -6), S(-15, -9), S(-9, 1),
    S(-11, -2), S(-12, -15), S(-22, -10), S(-12, -17), S(-29, -15), S(-21, -7), S(-23, -15), S(-21, 0),
    S(-8, -3), S(-14, -8), S(-4, -10), S(-27, -6), S(-27, -10), S(-9, -5), S(-2, -16), S(-12, 3),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};

const int32_t threats[6][6] = {

    {
        S(3, -9), S(11, -44), S(26, -30), S(-19, -27), S(-9, -62), S(-87, 2),
    },
    {
        S(-10, 4), S(0, 0), S(20, 22), S(48, -6), S(22, -34), S(0, 0),
    },
    {
        S(-6, 5), S(15, 19), S(0, 0), S(30, 4), S(33, 69), S(0, 0),
    },
    {
        S(-16, 12), S(2, 14), S(10, 11), S(0, 0), S(45, 0), S(0, 0),
    },
    {
        S(-4, 5), S(0, 6), S(-3, 27), S(2, -1), S(0, 0), S(0, 0),
    },
    {
        S(37, 33), S(-40, 8), S(-19, 21), S(-15, -2), S(0, 0), S(0, 0),
    },
};


const int32_t rook_semi_open[2] = {
    S(21, 9), S(53, 4),
};


const int32_t blocking_pawn[2] = {
    S(-7, -3), S(-3, -4),
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

                // Piece blocking pawn
                // Knights
                if (is_white ? (WHITE_FRONT_MASK[sq] & wn.getBits()) : (BLACK_FRONT_MASK[sq] & bn.getBits()))
                    eval_array[is_white ? 0 : 1] += blocking_pawn[0];
                // Bishops
                if (is_white ? (WHITE_FRONT_MASK[sq] & wb.getBits()) : (BLACK_FRONT_MASK[sq] & bb.getBits()))
                    eval_array[is_white ? 0 : 1] += blocking_pawn[1];

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
