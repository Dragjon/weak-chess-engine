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
// Double attacks
// Pieces are given bonuses for double attacking specific varieties of two opponent pieces
// This may be pieces worth more than pawn, minors or pieces worth more than bishop

const int32_t PSQT[6][64] = {
    {
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
        S(47, 178), S(77, 190), S(72, 176), S(82, 142), S(80, 119), S(80, 128), S(50, 149), S(33, 158),
        S(87, 159), S(95, 179), S(93, 166), S(104, 145), S(115, 108), S(156, 113), S(133, 156), S(105, 135),
        S(79, 127), S(93, 134), S(92, 123), S(77, 114), S(93, 101), S(96, 103), S(82, 119), S(78, 99),
        S(73, 103), S(78, 117), S(85, 103), S(83, 112), S(77, 101), S(87, 94), S(69, 106), S(70, 84),
        S(89, 97), S(99, 107), S(90, 104), S(64, 113), S(82, 110), S(87, 102), S(94, 101), S(81, 82),
        S(84, 99), S(113, 104), S(102, 100), S(83, 108), S(76, 122), S(96, 103), S(106, 101), S(76, 79),
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
    },
    {
        S(13, 81), S(20, 142), S(60, 130), S(88, 122), S(123, 118), S(49, 103), S(44, 137), S(63, 58),
        S(102, 143), S(107, 126), S(84, 125), S(101, 126), S(98, 116), S(115, 108), S(110, 119), S(128, 124),
        S(110, 119), S(105, 123), S(154, 168), S(162, 170), S(172, 164), S(196, 148), S(113, 118), S(127, 111),
        S(120, 133), S(106, 134), S(154, 177), S(171, 181), S(153, 182), S(175, 178), S(109, 138), S(147, 123),
        S(107, 134), S(95, 129), S(137, 178), S(136, 181), S(144, 184), S(142, 169), S(114, 126), S(118, 124),
        S(91, 120), S(82, 125), S(121, 161), S(128, 174), S(139, 171), S(127, 156), S(99, 117), S(108, 119),
        S(104, 144), S(90, 126), S(74, 123), S(89, 123), S(88, 120), S(83, 118), S(103, 115), S(128, 152),
        S(87, 137), S(114, 136), S(81, 123), S(93, 122), S(97, 123), S(101, 113), S(115, 143), S(114, 125)
    },
    {
        S(111, 171), S(90, 176), S(95, 167), S(57, 178), S(74, 173), S(81, 166), S(116, 163), S(80, 166),
        S(104, 163), S(117, 163), S(114, 163), S(107, 163), S(107, 157), S(120, 159), S(99, 171), S(104, 163),
        S(125, 175), S(137, 166), S(130, 164), S(133, 158), S(128, 163), S(150, 167), S(142, 166), S(130, 177),
        S(115, 172), S(121, 170), S(125, 167), S(142, 175), S(133, 169), S(129, 170), S(121, 167), S(118, 168),
        S(121, 169), S(110, 172), S(123, 168), S(135, 171), S(134, 167), S(129, 163), S(121, 166), S(129, 159),
        S(123, 169), S(135, 165), S(131, 165), S(130, 165), S(133, 168), S(135, 161), S(138, 157), S(143, 158),
        S(142, 169), S(138, 154), S(140, 151), S(128, 160), S(135, 159), S(138, 155), S(153, 158), S(145, 152),
        S(135, 158), S(145, 164), S(134, 160), S(123, 165), S(128, 163), S(126, 171), S(134, 157), S(153, 140)
    },
    {
        S(160, 305), S(152, 309), S(156, 317), S(155, 315), S(159, 307), S(182, 302), S(163, 306), S(183, 301),
        S(157, 301), S(159, 307), S(173, 313), S(188, 304), S(173, 306), S(199, 291), S(193, 288), S(206, 282),
        S(151, 296), S(177, 292), S(170, 296), S(174, 290), S(199, 282), S(205, 274), S(227, 271), S(197, 274),
        S(150, 300), S(161, 294), S(164, 299), S(170, 295), S(165, 287), S(178, 278), S(172, 282), S(173, 279),
        S(138, 298), S(133, 300), S(143, 299), S(152, 298), S(149, 297), S(144, 291), S(152, 284), S(148, 283),
        S(132, 299), S(132, 295), S(138, 296), S(143, 298), S(146, 294), S(148, 284), S(163, 271), S(150, 276),
        S(136, 295), S(140, 295), S(150, 295), S(152, 293), S(154, 288), S(154, 283), S(166, 274), S(144, 283),
        S(154, 302), S(151, 297), S(153, 302), S(159, 295), S(159, 292), S(150, 296), S(158, 287), S(156, 288)
    },
    {
        S(327, 567), S(320, 573), S(338, 588), S(365, 573), S(348, 576), S(360, 574), S(405, 525), S(360, 560),
        S(348, 553), S(335, 564), S(331, 595), S(319, 613), S(311, 632), S(358, 575), S(363, 569), S(389, 573),
        S(358, 552), S(351, 557), S(353, 578), S(350, 585), S(359, 588), S(383, 575), S(392, 551), S(378, 565),
        S(341, 570), S(349, 570), S(344, 576), S(343, 590), S(343, 592), S(350, 584), S(359, 586), S(357, 578),
        S(350, 562), S(339, 576), S(343, 572), S(347, 584), S(344, 581), S(346, 573), S(352, 568), S(356, 573),
        S(346, 547), S(353, 555), S(346, 567), S(343, 563), S(345, 568), S(348, 562), S(361, 543), S(356, 552),
        S(349, 551), S(352, 546), S(359, 540), S(354, 551), S(354, 552), S(353, 525), S(361, 505), S(373, 498),
        S(344, 554), S(345, 545), S(348, 546), S(353, 558), S(351, 537), S(331, 536), S(342, 526), S(355, 523)
    },
    {
        S(75, -119), S(73, -69), S(102, -52), S(-17, -8), S(-39, 4), S(-30, 5), S(40, -12), S(159, -114),
        S(-59, -28), S(43, -7), S(22, 9), S(110, 1), S(21, 40), S(16, 47), S(32, 30), S(-19, 8),
        S(-77, -19), S(78, -6), S(38, 20), S(20, 36), S(23, 60), S(60, 45), S(27, 32), S(-35, 10),
        S(-44, -27), S(-2, 0), S(-5, 25), S(-43, 49), S(-70, 76), S(-49, 60), S(-65, 38), S(-124, 22),
        S(-57, -30), S(-11, -6), S(-22, 20), S(-49, 41), S(-69, 73), S(-38, 50), S(-56, 32), S(-130, 21),
        S(-26, -34), S(22, -14), S(-5, 7), S(-14, 19), S(-24, 52), S(-18, 39), S(-1, 19), S(-51, 9),
        S(29, -45), S(31, -21), S(22, -9), S(-1, 0), S(-18, 35), S(-4, 26), S(25, 7), S(4, -9),
        S(13, -80), S(43, -60), S(25, -39), S(-38, -21), S(-5, -5), S(-33, 6), S(16, -17), S(0, -44)
    },
};


const int32_t mobilities[5][28] = {

    {
        S(0, 0), S(0, 0), S(115, 196), S(140, 193), S(163, 225), S(0, 0), S(198, 238), S(0, 0), S(175, 208), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(122, 144), S(136, 139), S(140, 169), S(154, 179), S(158, 188), S(169, 203), S(174, 210), S(180, 218), S(183, 222), S(186, 227), S(186, 225), S(188, 223), S(206, 220), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(0, 0), S(196, 304), S(201, 322), S(205, 330), S(208, 336), S(208, 340), S(208, 346), S(211, 348), S(214, 351), S(215, 356), S(217, 360), S(218, 366), S(221, 370), S(220, 375), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(0, 0), S(0, 0), S(444, 343), S(433, 425), S(460, 472), S(462, 495), S(458, 557), S(461, 571), S(458, 593), S(460, 600), S(461, 612), S(463, 624), S(465, 626), S(467, 631), S(467, 639), S(467, 645), S(468, 650), S(467, 656), S(467, 660), S(467, 668), S(469, 668), S(471, 668), S(479, 663), S(477, 665), S(491, 659), S(525, 638), S(575, 611),
    },
    {
        S(0, 0), S(0, 0), S(0, 0), S(48, -7), S(53, -16), S(37, 2), S(27, 0), S(24, -5), S(19, -3), S(15, -3), S(14, -3), S(6, 0), S(4, -1), S(-3, 1), S(-12, 3), S(-22, 5), S(-36, 6), S(-51, 6), S(-63, 7), S(-79, 6), S(-79, 2), S(-84, 0), S(-91, -2), S(-98, -7), S(-114, -7), S(-98, -19), S(-112, -17), S(-91, -30),
    },
};


const int32_t bishop_pair = S(19, 58);


const int32_t passed_pawns[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(47, 178), S(77, 190), S(72, 176), S(82, 142), S(80, 119), S(80, 128), S(50, 149), S(33, 158),
    S(16, 142), S(30, 143), S(21, 106), S(8, 72), S(12, 83), S(11, 97), S(-20, 106), S(-41, 140),
    S(14, 79), S(19, 76), S(19, 52), S(13, 47), S(2, 46), S(10, 52), S(0, 69), S(-11, 78),
    S(5, 45), S(1, 39), S(-9, 30), S(-2, 23), S(-11, 26), S(-5, 30), S(3, 42), S(0, 40),
    S(0, 9), S(-8, 18), S(-17, 13), S(-17, 8), S(-13, 7), S(-6, 6), S(8, 22), S(14, 7),
    S(-7, 7), S(-3, 11), S(-16, 12), S(-14, 6), S(1, -8), S(1, -2), S(19, 2), S(7, 7),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};

const int32_t inner_king_zone_attacks[4] = {
    S(10, -6), S(18, -4), S(20, -7), S(13, 7),
};


const int32_t outer_king_zone_attacks[4] = {
    S(0, 1), S(0, 0), S(4, -4), S(3, 1),
};


const int32_t doubled_pawn_penalty[8] = {
    S(-7, -39), S(-4, -26), S(-11, -24), S(-10, -10), S(-9, -19), S(-12, -22), S(-5, -24), S(-14, -34),
};


const int32_t pawn_storm[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(22, -59), S(-34, -53), S(-5, -92), S(-17, -48), S(-60, 48), S(-48, -10), S(-3, -10), S(-33, 0),
    S(-19, -33), S(-20, -41), S(10, -51), S(-1, -27), S(-10, 17), S(-30, 0), S(-44, -5), S(-44, 5),
    S(-30, -10), S(-29, -13), S(-22, -13), S(-3, -14), S(-8, 13), S(-20, 5), S(-34, 11), S(-47, 17),
    S(-32, -5), S(-25, -4), S(-22, -2), S(-8, -8), S(-5, 6), S(-23, 8), S(-28, 8), S(-43, 16),
    S(-45, 3), S(-44, 4), S(-24, 0), S(7, -5), S(-11, 1), S(-23, 3), S(-57, 17), S(-58, 23),
    S(-37, 8), S(-56, 10), S(-45, 9), S(-23, 3), S(-27, 4), S(-39, 8), S(-71, 27), S(-53, 37),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};

const int32_t isolated_pawns[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(44, -14), S(107, -93), S(47, 0), S(48, -1), S(16, 5), S(1, 28), S(-29, 27), S(-2, 4),
    S(11, -16), S(12, -41), S(1, -16), S(3, -5), S(1, -11), S(-5, -3), S(22, -21), S(8, -22),
    S(10, -8), S(1, -24), S(-5, -15), S(-6, -18), S(-5, -20), S(8, -17), S(22, -24), S(8, -6),
    S(0, 2), S(-1, -11), S(-16, -6), S(-18, -19), S(-12, -18), S(-12, -5), S(-10, -7), S(-7, 3),
    S(-12, -2), S(-12, -15), S(-23, -11), S(-15, -17), S(-30, -16), S(-22, -8), S(-22, -15), S(-21, 0),
    S(-10, -6), S(-16, -10), S(-5, -13), S(-28, -7), S(-28, -12), S(-10, -7), S(-4, -18), S(-13, 1),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};

const int32_t threats[6][6] = {

    {
        S(3, -10), S(11, -44), S(26, -29), S(-20, -26), S(-10, -62), S(-91, 2),
    },
    {
        S(-11, 4), S(0, 0), S(20, 22), S(36, -10), S(20, -44), S(0, 0),
    },
    {
        S(-6, 5), S(15, 19), S(0, 0), S(28, 2), S(33, 65), S(0, 0),
    },
    {
        S(-17, 12), S(1, 14), S(10, 11), S(0, 0), S(45, 0), S(0, 0),
    },
    {
        S(-4, 5), S(0, 5), S(-3, 26), S(2, -1), S(0, 0), S(0, 0),
    },
    {
        S(36, 31), S(-42, 8), S(-19, 20), S(-15, -2), S(0, 0), S(0, 0),
    },
};


const int32_t rook_semi_open[2] = {
    S(22, 9), S(55, 5),
};


const int32_t phalanx_pawns[8] = {
    S(0, 0), S(-2, -7), S(-1, -3), S(18, 11), S(47, 39), S(128, 167), S(-129, 413), S(0, 0),
};


const int32_t double_attacks[6] = {
    S(28, -4), S(32, 178), S(39, 147), S(-16, 15), S(0, 2), S(-10, 32),
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

    chess::Bitboard w_more_than_pawn = wn | wb | wr | wq | wk;
    chess::Bitboard b_more_than_pawn = bn | bb | br | bq | bk;
    chess::Bitboard w_more_than_bishop = wr | wq | wk;
    chess::Bitboard b_more_than_bishop = br | bq | bk;
    chess::Bitboard w_minors = wn | wb;
    chess::Bitboard b_minors = bn | bb;

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

            
            // Pawn double attacking > pawn
            if (j == 0 && __builtin_popcountll(is_white ? (attacks_bb & b_more_than_pawn.getBits()) : (attacks_bb & w_more_than_pawn.getBits())) == 2)
                eval_array[is_white ? 0 : 1] += double_attacks[0];

            // Minors double attacking > minors
            if ((j == 1 || j == 2) && __builtin_popcountll(is_white ? (attacks_bb & b_more_than_bishop.getBits()) : (attacks_bb & w_more_than_bishop.getBits())) >= 2)
                eval_array[is_white ? 0 : 1] += double_attacks[j];

            // Rook double attacking minors (because minors cannot attack rook on same ray)
            if (j == 3 && __builtin_popcountll(is_white ? (attacks_bb & b_minors.getBits()) : (attacks_bb & w_minors.getBits())) >= 2)
                eval_array[is_white ? 0 : 1]  += double_attacks[3];

            // Queen double attacking king and > pawn
            if (j == 4 && __builtin_popcountll(is_white ? (attacks_bb & (b_more_than_pawn.getBits() | bk.getBits())) : (attacks_bb & (w_more_than_pawn.getBits() | wk.getBits()))) >= 2)
                eval_array[is_white ? 0 : 1]  += double_attacks[4];

            // King double attacking minors
            if (j == 5 && __builtin_popcountll(is_white ? (attacks_bb & b_minors.getBits()) : (attacks_bb & w_minors.getBits())) == 2)
                eval_array[is_white ? 0 : 1]  += double_attacks[5];

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
