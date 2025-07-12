#pragma once
#include <cstdint>

int32_t count(uint64_t bb);
extern const uint64_t WHITE_PASSED_MASK[64];
extern const uint64_t BLACK_PASSED_MASK[64];
extern const uint64_t OUTER_2_SQ_RING_MASK[64];
bool is_white_passed_pawn(int32_t square, uint64_t black_pawns);
bool is_black_passed_pawn(int32_t square, uint64_t white_pawns);
int32_t count_white_passed_pawns(uint64_t white_pawns, uint64_t black_pawns);
int32_t count_black_passed_pawns(uint64_t black_pawns, uint64_t white_pawns);