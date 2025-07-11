#include <stdint.h>

#include "chess.hpp"
#include "packing.hpp"
#include "eval.hpp"
#include "defaults.hpp"

using namespace chess;
using namespace std;

// Entire evaluation function is tuned with non other than Gedas' Texel tuner <3
// https://github.com/GediminasMasaitis/texel-tuner

// Helper: LSB index
inline int lsb(uint64_t bb) {
    assert(bb != 0);
    return __builtin_ctzll(bb);
}

// Passed pawn masks
const uint64_t WHITE_PASSED_MASK[64] = {
    217020518514230016ull,    506381209866536704ull,
    1012762419733073408ull,    2025524839466146816ull,
    4051049678932293632ull,    8102099357864587264ull,
    16204198715729174528ull,    13889313184910721024ull,
    217020518514229248ull,    506381209866534912ull,
    1012762419733069824ull,    2025524839466139648ull,
    4051049678932279296ull,    8102099357864558592ull,
    16204198715729117184ull,    13889313184910671872ull,
    217020518514032640ull,    506381209866076160ull,
    1012762419732152320ull,    2025524839464304640ull,
    4051049678928609280ull,    8102099357857218560ull,
    16204198715714437120ull,    13889313184898088960ull,
    217020518463700992ull,    506381209748635648ull,
    1012762419497271296ull,    2025524838994542592ull,
    4051049677989085184ull,    8102099355978170368ull,
    16204198711956340736ull,    13889313181676863488ull,
    217020505578799104ull,    506381179683864576ull,
    1012762359367729152ull,    2025524718735458304ull,
    4051049437470916608ull,    8102098874941833216ull,
    16204197749883666432ull,    13889312357043142656ull,
    217017207043915776ull,    506373483102470144ull,
    1012746966204940288ull,    2025493932409880576ull,
    4050987864819761152ull,    8101975729639522304ull,
    16203951459279044608ull,    13889101250810609664ull,
    216172782113783808ull,    504403158265495552ull,
    1008806316530991104ull,    2017612633061982208ull,
    4035225266123964416ull,    8070450532247928832ull,
    16140901064495857664ull,    13835058055282163712ull,
    0ull,    0ull,
    0ull,    0ull,
    0ull,    0ull,
    0ull,    0ull
};

const uint64_t BLACK_PASSED_MASK[64] = {
    0ull,    0ull,
    0ull,    0ull,
    0ull,    0ull,
    0ull,    0ull,
    3ull,    7ull,
    14ull,    28ull,
    56ull,    112ull,
    224ull,    192ull,
    771ull,    1799ull,
    3598ull,    7196ull,
    14392ull,    28784ull,
    57568ull,    49344ull,
    197379ull,    460551ull,
    921102ull,    1842204ull,
    3684408ull,    7368816ull,
    14737632ull,    12632256ull,
    50529027ull,    117901063ull,
    235802126ull,    471604252ull,
    943208504ull,    1886417008ull,
    3772834016ull,    3233857728ull,
    12935430915ull,    30182672135ull,
    60365344270ull,    120730688540ull,
    241461377080ull,    482922754160ull,
    965845508320ull,    827867578560ull,
    3311470314243ull,    7726764066567ull,
    15453528133134ull,    30907056266268ull,
    61814112532536ull,    123628225065072ull,
    247256450130144ull,    211934100111552ull,
    847736400446211ull,    1978051601041159ull,
    3956103202082318ull,    7912206404164636ull,
    15824412808329272ull,    31648825616658544ull,
    63297651233317088ull,    54255129628557504ull
};

// Check if square is passed pawn for white
inline bool is_white_passed_pawn(int32_t square, uint64_t black_pawns) {
    return (WHITE_PASSED_MASK[square] & black_pawns) == 0;
}

// Check if square is passed pawn for black
inline bool is_black_passed_pawn(int32_t square, uint64_t white_pawns) {
    return (BLACK_PASSED_MASK[square] & white_pawns) == 0;
}

// Count passed pawns for white
int32_t count_white_passed_pawns(uint64_t white_pawns, uint64_t black_pawns) {
    int32_t count = 0;
    uint64_t bb = white_pawns;
    while (bb) {
        int sq = __builtin_ctzll(bb); // or lsb(bb)
        if (is_white_passed_pawn(sq, black_pawns))
            ++count;
        bb &= bb - 1; // Clear least significant bit
    }
    return count;
}

// Count passed pawns for black
int32_t count_black_passed_pawns(uint64_t black_pawns, uint64_t white_pawns) {
    int count = 0;
    uint64_t bb = black_pawns;
    while (bb) {
        int32_t sq = __builtin_ctzll(bb); // or lsb(bb)
        if (is_black_passed_pawn(sq, white_pawns))
            ++count;
        bb &= bb - 1;
    }
    return count;
}


enum Square {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8
};

// Piece Square Tables (which includes the material value of each piece)
// Each piece type has its own piece square table, whiched it used
// to evaluate its positioning on the board
const int32_t PSQT[6][64] = {
    {
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
        S(126, 265), S(157, 255), S(133, 257), S(156, 211), S(148, 206), S(131, 221), S(72, 260), S(38, 273),
        S(60, 204), S(77, 213), S(105, 180), S(108, 158), S(116, 150), S(132, 143),S(119, 184), S(72, 180),
        S(51, 136), S(74, 127), S(73, 111), S(79, 99), S(96, 93), S(86, 100), S(91,116), S(69, 114),
        S(42, 111), S(67, 112), S(69, 95), S(82, 91), S(85, 90), S(79, 91), S(79, 104), S(59, 95),
        S(45, 105), S(64, 110), S(66, 93), S(72, 100), S(81, 97), S(74, 93), S(93, 100), S(68, 92),
        S(47, 108), S(67, 113), S(63, 97), S(61, 109), S(72, 112), S(77, 101), S(97,100), S(60, 93),
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
    },

    {
        S(13, 76), S(21, 139), S(59, 121), S(95, 112), S(130, 115), S(82, 90), S(51,142), S(65, 50),
        S(117, 136), S(114, 123), S(114, 130), S(129, 129), S(112, 122), S(179, 108), S(123, 118), S(160, 118),
        S(112, 115), S(121, 130), S(152, 166), S(160, 168), S(200, 154), S(203, 147), S(149, 121), S(142, 104),
        S(114, 124), S(101, 146), S(137, 176), S(160, 178), S(139, 180), S(163, 176), S(113, 144), S(147, 116),
        S(101, 126), S(91, 135), S(123, 176), S(125, 176), S(135, 179), S(128, 168),S(109, 136), S(113, 117),
        S(85, 111), S(81, 129), S(115, 155), S(120, 168), S(130, 165), S(118, 149),S(100, 123), S(101, 111),
        S(98, 136), S(87, 116), S(78, 125), S(94, 126), S(92, 126), S(85, 122), S(97, 106), S(123, 144),
        S(66, 123), S(116, 127), S(79, 110), S(92, 113), S(97, 112), S(95, 104), S(115, 135), S(97, 109)
    },

    {
        S(115, 168), S(88, 175), S(94, 170), S(59, 180), S(84, 174), S(100, 164), S(116, 162), S(96, 164),
        S(121, 157), S(131, 159), S(125, 160), S(111, 161), S(139, 151), S(139, 153), S(140, 162), S(134, 154),
        S(130, 175), S(146, 162), S(141, 160), S(155, 150), S(148, 155), S(183, 157), S(167, 159), S(162, 170),
        S(128, 170), S(131, 168), S(142, 159), S(154, 169), S(148, 162), S(144, 164), S(133, 163), S(128, 168),
        S(128, 164), S(124, 166), S(125, 166), S(144, 162), S(140, 162), S(126, 160), S(122, 163), S(139, 156),
        S(137, 166), S(135, 164), S(133, 161), S(131, 159), S(131, 162), S(134, 159), S(137, 155), S(149, 159),
        S(147, 168), S(142, 155), S(143, 146), S(125, 158), S(132, 156), S(135, 152), S(150, 160), S(146, 153),
        S(138, 159), S(150, 165), S(139, 156), S(123, 161), S(131, 161), S(124, 168), S(142, 153), S(149, 146)
    },

    {
        S(200, 302), S(183, 310), S(185, 317), S(188, 312), S(205, 304), S(225, 298), S(219, 299), S(233, 295),
        S(177, 306), S(173, 318), S(191, 321), S(209, 311), S(197, 311), S(229, 299), S(222, 295), S(246, 282),
        S(159, 306), S(180, 306), S(175, 308), S(180, 305), S(210, 293), S(213, 287), S(257, 278), S(231, 274),
        S(153, 306), S(162, 302), S(161, 310), S(167, 305), S(169, 293), S(180, 286), S(189, 283), S(191, 279),
        S(140, 298), S(138, 301), S(148, 301), S(157, 298), S(157, 294), S(152, 290), S(168, 282), S(161, 281),
        S(137, 295), S(141, 292), S(149, 290), S(154, 290), S(159, 285), S(161, 277), S(184, 262), S(167, 268),
        S(138, 290), S(145, 290), S(160, 288), S(160, 289), S(165, 281), S(159, 278), S(174, 271), S(152, 279),
        S(154, 292), S(156, 291), S(164, 295), S(167, 290), S(169, 282), S(154, 287), S(170, 280), S(156, 281)
    },

    {
        S(370, 567), S(352, 588), S(371, 606), S(401, 591), S(400, 595), S(405, 593), S(447, 537), S(404, 568),
        S(376, 558), S(349, 588), S(352, 617), S(344, 632), S(348, 651), S(389, 613), S(384, 591), S(427, 585),
        S(376, 561), S(369, 566), S(364, 598), S(372, 606), S(386, 615), S(421, 600), S(435, 566), S(435, 571),
        S(362, 567), S(362, 574), S(360, 585), S(360, 600), S(360, 614), S(374, 607), S(382, 601), S(390, 596),
        S(365, 563), S(356, 583), S(359, 579), S(364, 593), S(364, 590), S(365, 586), S(373, 584), S(383, 583),
        S(366, 553), S(369, 561), S(365, 570), S(366, 562), S(366, 567), S(373, 565), S(384, 554), S(382, 559),
        S(374, 549), S(371, 545), S(378, 539), S(380, 541), S(377, 546), S(379, 521), S(386, 502), S(410, 491),
        S(369, 548), S(369, 542), S(374, 537), S(381, 547), S(379, 524), S(354, 537), S(376, 521), S(382, 520)
    },

    {
        S(35, -93), S(20, -40), S(63, -31), S(-77, 17), S(-28, 0), S(-9, 2), S(26, -9), S(110, -110),
        S(-104, 5), S(-15, 34), S(-44, 48), S(62, 31), S(7, 50), S(1, 61), S(15, 49), S(-48, 18),
        S(-107, 18), S(29, 40), S(-10, 67), S(-25, 79), S(9, 78), S(63, 69), S(16, 58), S(-43, 23),
        S(-69, 5), S(-52, 45), S(-54, 74), S(-83, 96), S(-79, 95), S(-56, 78), S(-75, 59), S(-130, 30),
        S(-75, -3), S(-54, 32), S(-62, 64), S(-85, 87), S(-84, 86), S(-59, 61), S(-83, 42), S(-137, 23),
        S(-46, -9), S(-10, 17), S(-42, 44), S(-48, 56), S(-45, 54), S(-46, 43), S(-25, 18), S(-63, 3),
        S(22, -28), S(5, 1), S(0, 15), S(-27, 27), S(-29, 28), S(-15, 17), S(19, -4), S(9, -24),
        S(4, -61), S(45, -43), S(29, -19), S(-44, -1), S(10, -22), S(-28, -8), S(29,-36), S(10, -63)
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
        S(0, 0), S(0, 0), S(117, 193), S(131, 184), S(154, 218), S(0, 0), S(181, 218), S(0, 0), S(168, 198), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },

    {
        S(0, 0), S(104, 119), S(117, 123), S(123, 154), S(136, 163), S(142, 171), S(154, 187), S(161, 192), S(168, 203), S(169, 207), S(175, 213), S(177, 209), S(178, 210), S(205, 200), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },

    {
        S(0, 0), S(0, 0), S(177, 298), S(184, 309), S(191, 317), S(195, 320), S(196, 323), S(199, 329), S(205, 331), S(211, 334), S(217, 338), S(224, 340), S(228, 344), S(236, 346), S(240, 348), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },

    {
        S(0, 0), S(0, 0), S(0, 0), S(392, 382), S(372, 403), S(396, 454), S(400, 471), S(398, 533), S(404, 545), S(404, 566), S(407, 573), S(409, 585), S(411, 596), S(414, 599), S(417, 604), S(418, 612), S(419, 618), S(421, 625), S(421, 631), S(423, 635), S(425, 644), S(428, 643), S(430, 645), S(439, 642), S(439, 641), S(452, 639), S(491, 618), S(544, 597),
    },

    {
        S(0, 0), S(0, 0), S(0, 0), S(56, -14), S(64, -16), S(44, 3), S(35, 3), S(29, 0), S(23, 0), S(17, 0), S(16, 0), S(4, 5), S(1, 1), S(-7, 5), S(-18, 7), S(-32, 9), S(-48, 10), S(-65, 9), S(-80, 10), S(-97, 8), S(-101, 3), S(-106, 0), S(-113, -5), S(-124, -11), S(-137, -16), S(-125, -30), S(-127, -34), S(-120, -46),
    },

};

// Bishop pair evaluation
const int32_t bishop_pair = S(18, 62);

// Passed pawn bonus
const int32_t passed_pawns[9]{
    S(17, -20), S(9, 6), S(-5, 34), S(-18, 47), S(-25, 29), S(-27, 33), S(-66, 26), S(21, -42), S(-147, -60),
};

// Knight on C file blocking c-pawn malus
const int32_t knight_blocking_c_pawn_malus = S(-12, 4);

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

            // Mobilities for knight - queen, and king virtual mobility
            if (j > 0){
                int32_t attacks = 0;
                switch (j)
                {
                    // knights
                    case 1:
                        attacks = chess::attacks::knight(static_cast<chess::Square>(sq)).count();
                        if (is_white ? (sq == C3 && board.at(C2) == chess::Piece::WHITEPAWN) : (sq == C6 && board.at(C7) == chess::Piece::BLACKPAWN)){
                            score_array[is_white ? 0 : 1] += knight_blocking_c_pawn_malus;
                        }
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
                    // king virtual mobility
                    case 5:
                        attacks = chess::attacks::queen(static_cast<chess::Square>(sq), board.occ()).count();
                        break;

                }
                score_array[is_white ? 0 : 1] += mobilities[j-1][attacks];
            }
        }

    }

    // Bishop Pair
    if (wb.count() == 2) score_array[0] += bishop_pair;
    if (bb.count() == 2) score_array[1] += bishop_pair;

    // Passed pawn bonus
    int64_t wp_bits = wp.getBits();
    int64_t bp_bits = bp.getBits();

    score_array[0] += passed_pawns[count_white_passed_pawns(wp_bits, bp_bits)];
    score_array[1] += passed_pawns[count_black_passed_pawns(bp_bits, wp_bits)];

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
