#include <stdint.h>

#include "chess.hpp"
#include "packing.hpp"
#include "eval.hpp"
#include "defaults.hpp"

using namespace chess;
using namespace std;

const int32_t PSQT[6][64] = {
    {
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
        S(117, 286), S(146, 278), S(122, 279), S(147, 230), S(134, 226), S(118, 239), S(54, 283), S(28, 295),
        S(53, 224), S(69, 233), S(96, 201), S(100, 180), S(104, 171), S(124, 157), S(110, 202), S(68, 199),
        S(43, 155), S(67, 144), S(67, 126), S(72, 116), S(90, 108), S(80, 112), S(88, 129), S(65, 130),
        S(36, 130), S(62, 127), S(63, 110), S(77, 107), S(80, 105), S(75, 105), S(76, 118), S(55, 110),
        S(39, 123), S(58, 125), S(61, 108), S(67, 118), S(80, 112), S(71, 109), S(92, 116), S(65, 106),
        S(41, 126), S(62, 128), S(56, 115), S(61, 126), S(71, 127), S(85, 114), S(102, 114), S(58, 106),
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
    },

    {
    S(11, 63), S(16, 140), S(50, 117), S(82, 109), S(116, 111), S(62, 87), S(40, 145), S(57, 43),
        S(116, 132), S(109, 116), S(102, 122), S(119, 121), S(103, 115), S(161, 101), S(111, 111), S(157, 116),
        S(108, 110), S(110, 124), S(149, 164), S(157, 165), S(193, 151), S(197, 145), S(135, 114), S(138, 100),
        S(108, 120), S(91, 139), S(134, 175), S(156, 176), S(137, 177), S(161, 173), S(103, 138), S(143, 112),
        S(96, 121), S(81, 129), S(120, 175), S(123, 175), S(133, 178), S(126, 168), S(100, 131), S(107, 113),
        S(80, 105), S(71, 123), S(109, 154), S(118, 169), S(131, 166), S(117, 149), S(92, 118), S(96, 107),
        S(97, 134), S(83, 112), S(67, 120), S(87, 121), S(85, 122), S(86, 117), S(100, 103), S(121, 144),
        S(61, 113), S(116, 125), S(78, 104), S(92, 108), S(95, 110), S(101, 100), S(117, 132), S(92, 101)
    },

    {
    S(125, 169), S(92, 177), S(100, 172), S(61, 184), S(81, 177), S(95, 167), S(118, 164), S(100, 167),
        S(128, 158), S(140, 160), S(132, 161), S(116, 163), S(143, 154), S(140, 153), S(140, 163), S(138, 157),
        S(138, 180), S(153, 164), S(148, 162), S(164, 152), S(152, 157), S(189, 159), S(172, 161), S(171, 173),
        S(135, 173), S(139, 171), S(150, 161), S(160, 172), S(156, 165), S(151, 166), S(142, 167), S(135, 174),
        S(136, 168), S(132, 169), S(132, 169), S(153, 165), S(150, 165), S(134, 164), S(130, 167), S(148, 159),
        S(145, 170), S(143, 167), S(143, 165), S(140, 164), S(143, 167), S(144, 164), S(146, 159), S(158, 161),
        S(155, 170), S(151, 159), S(152, 150), S(137, 164), S(143, 162), S(155, 155), S(167, 164), S(155, 155),
        S(147, 164), S(159, 168), S(152, 161), S(137, 167), S(145, 166), S(142, 173), S(161, 154), S(158, 152)
    },

    {
    S(194, 299), S(175, 308), S(175, 317), S(176, 312), S(192, 305), S(212, 298), S(213, 296), S(231, 291),
        S(174, 302), S(168, 314), S(185, 318), S(204, 308), S(191, 308), S(217, 296), S(215, 291), S(245, 277),
        S(155, 302), S(175, 304), S(170, 306), S(173, 304), S(200, 291), S(206, 285), S(252, 276), S(232, 272),
        S(149, 304), S(157, 302), S(155, 310), S(160, 305), S(164, 292), S(173, 286), S(185, 284), S(189, 280),
        S(137, 297), S(134, 300), S(143, 300), S(151, 299), S(153, 295), S(146, 292), S(166, 283), S(158, 280),
        S(135, 293), S(137, 291), S(145, 289), S(150, 291), S(157, 286), S(158, 278), S(183, 263), S(166, 265),
        S(136, 287), S(142, 289), S(157, 288), S(158, 289), S(163, 282), S(166, 276), S(179, 269), S(154, 276),
        S(153, 288), S(155, 289), S(164, 293), S(171, 288), S(175, 282), S(169, 285), S(178, 277), S(157, 276)
    },

    {
    S(362, 565), S(341, 589), S(360, 606), S(389, 593), S(393, 592), S(397, 592), S(438, 537), S(394, 570),
        S(369, 555), S(343, 583), S(345, 614), S(337, 631), S(342, 648), S(378, 611), S(369, 596), S(418, 587),
        S(370, 561), S(362, 567), S(356, 600), S(365, 606), S(375, 617), S(417, 599), S(427, 570), S(430, 574),
        S(355, 568), S(354, 576), S(353, 585), S(352, 601), S(352, 615), S(367, 608), S(375, 604), S(383, 598),
        S(359, 562), S(350, 582), S(351, 580), S(358, 593), S(358, 590), S(358, 589), S(367, 586), S(376, 584),
        S(359, 553), S(362, 560), S(360, 568), S(360, 565), S(363, 569), S(368, 569), S(378, 558), S(377, 558),
        S(369, 546), S(365, 545), S(373, 539), S(377, 542), S(374, 548), S(384, 523), S(389, 504), S(408, 486),
        S(365, 543), S(366, 539), S(373, 536), S(381, 551), S(378, 534), S(363, 539), S(386, 517), S(382, 516)
    },

    {
    S(29, -80), S(12, -30), S(48, -21), S(-98, 27), S(-50, 10), S(0, 11), S(39, 4), S(151, -104),
        S(-96, 13), S(-47, 40), S(-86, 52), S(17, 35), S(-37, 56), S(-33, 67), S(-4, 57), S(-28, 27),
        S(-116, 28), S(-8, 46), S(-74, 65), S(-91, 75), S(-54, 75), S(15, 68), S(-6, 66), S(-44, 37),
        S(-86, 18), S(-94, 51), S(-111, 70), S(-152, 82), S(-143, 82), S(-107, 76), S(-108, 67), S(-135, 42),
        S(-84, 6), S(-89, 37), S(-119, 60), S(-148, 75), S(-148, 75), S(-110, 61), S(-115, 50), S(-140, 34),
        S(-43, -2), S(-28, 20), S(-82, 41), S(-94, 52), S(-89, 52), S(-86, 43), S(-44, 24), S(-61, 13),
        S(38, -22), S(0, 3), S(-13, 15), S(-44, 25), S(-47, 29), S(-29, 19), S(14, 1), S(21, -15),
        S(29, -57), S(59, -39), S(36, -19), S(-51, -2), S(6, -22), S(-29, -5), S(40, -31), S(37, -60)
    },


};

const int32_t mobilities[4][28]{
    {
        S(0, 0), S(0, 0), S(120, 200), S(132, 184), S(158, 222), S(0, 0), S(189, 223), S(0, 0), S(168, 199), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },

    {
        S(0, 0), S(112, 112), S(125, 124), S(132, 156), S(144, 164), S(149, 174), S(161, 189), S(169, 195), S(176, 206), S(177, 209), S(182, 216), S(184, 212), S(185, 213), S(211, 203), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },

    {
        S(0, 0), S(0, 0), S(174, 296), S(183, 308), S(190, 315), S(195, 318), S(197, 322), S(200, 327), S(206, 329), S(212, 332), S(219, 336), S(226, 338), S(230, 342), S(238, 345), S(243, 346), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },

    {
        S(0, 0), S(0, 0), S(0, 0), S(401, 278), S(359, 420), S(389, 456), S(390, 483), S(390, 540), S(394, 553), S(395, 570), S(399, 576), S(402, 586), S(404, 597), S(407, 600), S(410, 605), S(411, 614), S(412, 621), S(414, 628), S(414, 634), S(415, 639), S(417, 648), S(421, 647), S(422, 650), S(431, 647), S(433, 645), S(446, 645), S(486, 623), S(539, 603),
    },

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

            // Mobilities for knight - queen
            if (j > 0 && j < 5){
                int32_t attacks = 0;
                switch (j)
                {
                    // knights
                    case 1:
                        attacks = chess::attacks::knight(static_cast<chess::Square>(sq)).count();
                        break;
                    // bishops
                    case 2:
                        attacks = chess::attacks::bishop(static_cast<chess::Square>(sq), board.occ()).count();
                        break;
                    // rooks
                    case 3:
                        attacks = chess::attacks::rook(static_cast<chess::Square>(sq), board.occ()).count();
                        break;
                    // queens
                    case 4:
                        attacks = chess::attacks::queen(static_cast<chess::Square>(sq), board.occ()).count();
                        break;

                }
                score_array[is_white ? 0 : 1] += mobilities[j-1][attacks];
            }
        }

    }

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
