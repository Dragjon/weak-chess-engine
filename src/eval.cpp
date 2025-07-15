// error 0.0764196

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
// own king tropism
// This refers to bonuses given by the chebyshev distance between our king and our 
// pieces. It may be better for some pieces to stay near the king to protect it in
// the midgame and vice versa for the endgame

const int32_t PSQT[6][64] = {
    {
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
        S(27, 157), S(59, 172), S(49, 154), S(58, 119), S(59, 94), S(59, 105), S(32, 127), S(16, 134),
        S(48, 109), S(61, 130), S(56, 117), S(62, 100), S(80, 58), S(111, 65), S(98, 106), S(69, 86),
        S(45, 79), S(65, 86), S(63, 73), S(35, 68), S(59, 53), S(59, 57), S(53, 70), S(44, 50),
        S(43, 55), S(52, 69), S(60, 54), S(47, 65), S(50, 53), S(53, 48), S(41, 58), S(37, 37),
        S(59, 47), S(70, 57), S(60, 52), S(23, 64), S(52, 58), S(49, 53), S(64, 51), S(51, 32),
        S(53, 45), S(83, 49), S(71, 45), S(42, 60), S(45, 68), S(59, 51), S(73, 46), S(44, 27),
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
    },
    {
        S(-17, 37), S(-22, 85), S(23, 76), S(50, 67), S(84, 64), S(8, 50), S(1, 78), S(33, 15),
        S(57, 85), S(65, 74), S(48, 76), S(68, 75), S(62, 66), S(85, 57), S(66, 67), S(80, 67),
        S(70, 66), S(69, 72), S(110, 107), S(117, 111), S(131, 103), S(152, 87), S(76, 67), S(85, 59),
        S(81, 78), S(69, 84), S(109, 116), S(126, 121), S(108, 121), S(130, 117), S(72, 87), S(105, 69),
        S(67, 80), S(60, 79), S(94, 117), S(93, 121), S(100, 123), S(98, 108), S(76, 76), S(77, 70),
        S(50, 67), S(45, 74), S(78, 100), S(85, 113), S(95, 110), S(83, 94), S(61, 67), S(67, 66),
        S(59, 86), S(49, 73), S(37, 72), S(54, 71), S(52, 70), S(48, 67), S(62, 62), S(81, 94),
        S(52, 95), S(70, 77), S(40, 70), S(54, 67), S(59, 68), S(62, 59), S(70, 84), S(80, 82)
    },
    {
        S(68, 110), S(48, 114), S(52, 107), S(16, 118), S(30, 112), S(38, 105), S(71, 102), S(36, 105),
        S(61, 101), S(73, 103), S(71, 102), S(64, 102), S(66, 97), S(77, 98), S(56, 111), S(57, 102),
        S(82, 115), S(94, 104), S(89, 104), S(92, 97), S(85, 103), S(111, 106), S(97, 105), S(88, 116),
        S(73, 111), S(77, 110), S(84, 105), S(100, 115), S(92, 108), S(86, 110), S(78, 107), S(73, 108),
        S(76, 109), S(69, 112), S(80, 108), S(94, 110), S(92, 106), S(87, 102), S(77, 106), S(85, 99),
        S(85, 107), S(90, 105), S(90, 105), S(87, 105), S(91, 107), S(93, 100), S(95, 96), S(101, 98),
        S(99, 108), S(98, 93), S(96, 90), S(87, 100), S(92, 98), S(97, 93), S(112, 97), S(103, 92),
        S(90, 98), S(100, 104), S(95, 99), S(80, 104), S(88, 102), S(86, 110), S(92, 95), S(109, 81)
    },
    {
        S(114, 203), S(102, 207), S(106, 214), S(104, 213), S(108, 205), S(133, 200), S(113, 205), S(135, 201),
        S(112, 197), S(105, 205), S(119, 210), S(132, 201), S(118, 204), S(144, 189), S(141, 187), S(160, 180),
        S(104, 193), S(124, 188), S(116, 192), S(117, 187), S(141, 179), S(146, 173), S(173, 168), S(149, 171),
        S(102, 197), S(109, 191), S(112, 195), S(117, 192), S(114, 183), S(124, 175), S(121, 179), S(124, 177),
        S(88, 196), S(86, 197), S(95, 195), S(105, 193), S(104, 192), S(96, 187), S(107, 179), S(102, 180),
        S(82, 197), S(89, 190), S(96, 190), S(99, 192), S(103, 188), S(103, 180), S(121, 164), S(105, 172),
        S(82, 192), S(93, 190), S(109, 188), S(109, 188), S(112, 182), S(109, 176), S(123, 168), S(94, 178),
        S(97, 198), S(100, 193), S(111, 195), S(118, 190), S(117, 186), S(103, 188), S(112, 181), S(102, 182)
    },
    {
        S(193, 363), S(185, 358), S(201, 381), S(230, 368), S(212, 371), S(221, 373), S(263, 315), S(221, 360),
        S(217, 338), S(198, 358), S(197, 387), S(184, 406), S(178, 425), S(224, 368), S(225, 366), S(251, 361),
        S(223, 345), S(217, 348), S(216, 372), S(215, 376), S(221, 382), S(249, 370), S(257, 345), S(242, 361),
        S(208, 364), S(214, 362), S(210, 367), S(208, 382), S(208, 384), S(215, 377), S(223, 382), S(221, 374),
        S(216, 355), S(205, 370), S(207, 365), S(214, 375), S(209, 375), S(211, 365), S(215, 361), S(223, 366),
        S(210, 344), S(218, 348), S(213, 361), S(208, 356), S(211, 359), S(214, 353), S(226, 334), S(220, 346),
        S(211, 337), S(214, 345), S(224, 335), S(221, 345), S(220, 344), S(221, 313), S(226, 296), S(239, 283),
        S(204, 354), S(205, 334), S(214, 342), S(220, 350), S(221, 328), S(198, 327), S(205, 317), S(217, 321)
    },
    {
        S(91, -125), S(75, -70), S(103, -52), S(-14, -7), S(-34, 2), S(-39, 3), S(32, -17), S(165, -124),
        S(-51, -32), S(45, -9), S(22, 8), S(106, 3), S(20, 40), S(13, 45), S(27, 27), S(-23, 3),
        S(-78, -20), S(81, -8), S(37, 20), S(18, 36), S(19, 59), S(63, 44), S(29, 28), S(-39, 8),
        S(-39, -29), S(-4, 0), S(-11, 26), S(-45, 48), S(-71, 74), S(-52, 59), S(-66, 36), S(-124, 20),
        S(-59, -30), S(-16, -5), S(-26, 20), S(-51, 42), S(-71, 71), S(-44, 49), S(-62, 32), S(-131, 19),
        S(-32, -33), S(17, -14), S(-8, 8), S(-19, 20), S(-31, 51), S(-23, 39), S(-5, 17), S(-56, 7),
        S(28, -46), S(29, -20), S(18, -9), S(-2, 0), S(-23, 33), S(-9, 24), S(24, 5), S(5, -12),
        S(9, -78), S(43, -60), S(24, -40), S(-39, -21), S(-10, -7), S(-34, 3), S(18, -20), S(-4, -42)
    },
};


const int32_t mobilities[5][28] = {

    {
        S(0, 0), S(0, 0), S(50, 104), S(89, 116), S(107, 143), S(0, 0), S(137, 153), S(0, 0), S(122, 134), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(67, 66), S(83, 62), S(87, 93), S(100, 102), S(105, 111), S(116, 126), S(121, 133), S(127, 141), S(129, 146), S(131, 151), S(131, 149), S(134, 147), S(152, 144), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(0, 0), S(133, 180), S(137, 196), S(142, 204), S(146, 210), S(146, 215), S(148, 220), S(152, 223), S(157, 227), S(161, 232), S(165, 236), S(168, 242), S(174, 247), S(175, 252), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    },
    {
        S(0, 0), S(0, 0), S(0, 0), S(270, 183), S(268, 186), S(291, 247), S(292, 270), S(289, 328), S(292, 342), S(291, 361), S(294, 368), S(295, 380), S(297, 391), S(299, 393), S(301, 398), S(301, 406), S(301, 412), S(302, 418), S(301, 424), S(302, 427), S(303, 435), S(305, 434), S(306, 434), S(315, 428), S(314, 429), S(329, 422), S(364, 400), S(412, 373),
    },
    {
        S(0, 0), S(0, 0), S(0, 0), S(41, -2), S(46, -9), S(31, 1), S(25, 1), S(23, -4), S(19, -2), S(15, -2), S(14, -3), S(6, 1), S(5, -1), S(-2, 1), S(-11, 3), S(-21, 5), S(-34, 6), S(-50, 6), S(-62, 6), S(-77, 6), S(-77, 2), S(-82, 0), S(-88, -1), S(-97, -6), S(-112, -6), S(-96, -18), S(-112, -16), S(-87, -30),
    },
};


const int32_t bishop_pair = S(19, 58);


const int32_t passed_pawns[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(27, 157), S(59, 172), S(49, 154), S(58, 119), S(59, 94), S(59, 105), S(32, 127), S(16, 134),
    S(20, 143), S(30, 150), S(19, 114), S(5, 74), S(10, 87), S(15, 101), S(-15, 113), S(-39, 143),
    S(15, 79), S(19, 76), S(18, 53), S(15, 48), S(5, 46), S(12, 53), S(1, 71), S(-9, 79),
    S(5, 44), S(2, 38), S(-10, 30), S(-3, 22), S(-11, 25), S(-3, 29), S(5, 40), S(3, 39),
    S(1, 8), S(-5, 16), S(-16, 13), S(-16, 7), S(-12, 6), S(-2, 4), S(12, 20), S(16, 6),
    S(-6, 8), S(-2, 11), S(-16, 13), S(-12, 5), S(1, -7), S(4, -2), S(22, 3), S(8, 7),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};

const int32_t inner_king_zone_attacks[4] = {
    S(10, -6), S(19, -4), S(21, -7), S(13, 6),
};


const int32_t outer_king_zone_attacks[4] = {
    S(0, 1), S(0, 0), S(3, -3), S(2, 0),
};


const int32_t doubled_pawn_penalty[8] = {
    S(0, -40), S(-2, -26), S(-5, -24), S(-5, -10), S(-2, -20), S(-4, -23), S(0, -25), S(-7, -36),
};


const int32_t pawn_storm[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(15, -59), S(-39, -52), S(-3, -91), S(-14, -50), S(-57, 48), S(-41, -12), S(-4, -9), S(-42, 2),
    S(-21, -33), S(-19, -42), S(11, -52), S(7, -29), S(-8, 17), S(-24, -1), S(-42, -8), S(-53, 5),
    S(-33, -9), S(-29, -14), S(-22, -13), S(5, -16), S(-6, 12), S(-11, 1), S(-32, 9), S(-51, 17),
    S(-38, -3), S(-25, -4), S(-21, -3), S(4, -11), S(-2, 5), S(-14, 4), S(-28, 8), S(-50, 17),
    S(-50, 5), S(-47, 5), S(-25, 0), S(19, -9), S(-11, 1), S(-18, 1), S(-59, 17), S(-66, 25),
    S(-42, 10), S(-56, 11), S(-47, 11), S(-13, 0), S(-27, 4), S(-38, 7), S(-71, 27), S(-60, 38),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};

const int32_t isolated_pawns[64] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
    S(47, -25), S(110, -107), S(52, -8), S(52, -6), S(21, 3), S(5, 26), S(-27, 22), S(0, 1),
    S(13, -18), S(12, -48), S(2, -24), S(3, -12), S(2, -17), S(-2, -8), S(20, -27), S(8, -25),
    S(8, -9), S(-2, -27), S(-8, -17), S(-8, -21), S(-5, -23), S(5, -20), S(17, -27), S(4, -7),
    S(-2, 1), S(-8, -12), S(-22, -6), S(-23, -19), S(-16, -19), S(-17, -7), S(-16, -8), S(-10, 1),
    S(-12, -2), S(-14, -15), S(-24, -10), S(-13, -16), S(-30, -15), S(-20, -8), S(-26, -14), S(-22, 0),
    S(-8, -3), S(-16, -7), S(-4, -10), S(-28, -6), S(-28, -10), S(-9, -5), S(-3, -15), S(-12, 3),
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
};

const int32_t threats[6][6] = {

    {
        S(3, -9), S(11, -44), S(26, -30), S(-21, -26), S(-11, -61), S(-86, 3),
    },
    {
        S(-10, 4), S(0, 0), S(20, 22), S(47, -5), S(22, -33), S(0, 0),
    },
    {
        S(-6, 5), S(15, 19), S(0, 0), S(30, 4), S(34, 68), S(0, 0),
    },
    {
        S(-5, 12), S(8, 15), S(16, 12), S(0, 0), S(52, 0), S(0, 0),
    },
    {
        S(-4, 4), S(0, 5), S(-2, 26), S(2, -2), S(0, 0), S(0, 0),
    },
    {
        S(36, 33), S(-41, 8), S(-20, 21), S(-14, -2), S(0, 0), S(0, 0),
    },
};


const int32_t own_king_tropism[6][8] = {

    {
        S(36, 45), S(27, 51), S(27, 53), S(29, 51), S(32, 50), S(26, 50), S(26, 50), S(26, 44),
    },
    {
        S(101, 131), S(97, 133), S(94, 134), S(94, 135), S(96, 134), S(96, 132), S(96, 133), S(96, 135),
    },
    {
        S(105, 133), S(95, 137), S(97, 134), S(93, 134), S(98, 135), S(89, 136), S(96, 136), S(95, 134),
    },
    {
        S(109, 233), S(112, 234), S(114, 234), S(113, 232), S(112, 232), S(115, 229), S(119, 228), S(121, 224),
    },
    {
        S(312, 424), S(292, 445), S(295, 440), S(292, 436), S(295, 435), S(298, 427), S(306, 441), S(305, 418),
    },
    {
        S(-2, -3), S(3, -5), S(6, -8), S(4, -5), S(-8, 1), S(22, 0), S(-8, 5), S(6, 28),
    },
};

// Gets the chebyshev distance given two points
int32_t get_chebychev_distance(int32_t row, int32_t col, int32_t row1, int32_t col1){
    return max(abs(row - row1), abs(col-col1));
}

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

    int32_t white_king_sq = board.kingSq(chess::Color::WHITE).index();
    int32_t black_king_sq = board.kingSq(chess::Color::BLACK).index();

    uint64_t not_kingside_w_mask = NOT_KINGSIDE_HALF_MASK[white_king_sq];
    uint64_t not_kingside_b_mask = NOT_KINGSIDE_HALF_MASK[black_king_sq];

    uint64_t white_king_2_sq_mask = OUTER_2_SQ_RING_MASK[white_king_sq];
    uint64_t black_king_2_sq_mask = OUTER_2_SQ_RING_MASK[black_king_sq];
    uint64_t white_king_inner_sq_mask = chess::attacks::king(white_king_sq).getBits();
    uint64_t black_king_inner_sq_mask = chess::attacks::king(black_king_sq).getBits();

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

            // Own king tropism
            eval_array[is_white ? 0 : 1] += own_king_tropism[j][get_chebychev_distance(is_white ? (white_king_sq / 8) : (black_king_sq / 8), sq / 8, is_white ? (white_king_sq % 8) : (black_king_sq % 8), sq % 8)];


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
                        attacks = count(attacks_bb);
                        break;
                    // bishops
                    case 2:
                        attacks_bb = chess::attacks::bishop(static_cast<chess::Square>(sq), board.occ()).getBits();
                        attacks = count(attacks_bb);
                        break;
                    // rooks
                    case 3:
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
            }

            // Actual king attacks because we let attacks bb for king be a queen attacks bb
            // for virtual mobility above
            if (j == 5) attacks_bb = chess::attacks::king(static_cast<chess::Square>(sq)).getBits();

            // Threats
            int32_t num_pawn_attacks = is_white ? count(attacks_bb & bp.getBits()) : count(attacks_bb & wp.getBits());
            int32_t num_knight_attacks = is_white ? count(attacks_bb & bn.getBits()) : count(attacks_bb & wn.getBits());
            int32_t num_bishop_attacks = is_white ? count(attacks_bb & bb.getBits()) : count(attacks_bb & wb.getBits());
            int32_t num_rook_attacks = is_white ? count(attacks_bb & br.getBits()) : count(attacks_bb & wr.getBits());
            int32_t num_queen_attacks = is_white ? count(attacks_bb & bq.getBits()) : count(attacks_bb & wq.getBits());
            int32_t num_king_attacks = is_white ? count(attacks_bb & bk.getBits()) : count(attacks_bb & wk.getBits());

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
