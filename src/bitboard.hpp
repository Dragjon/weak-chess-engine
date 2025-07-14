#pragma once
#include <cstdint>

int32_t count(uint64_t bb);
extern const uint64_t WHITE_PASSED_MASK[64];
extern const uint64_t BLACK_PASSED_MASK[64];
extern const uint64_t OUTER_2_SQ_RING_MASK[64];
extern const uint64_t WHITE_AHEAD_MASK[64];
extern const uint64_t BLACK_AHEAD_MASK[64];
extern const uint64_t WHITE_FRONT_MASK[64];
extern const uint64_t BLACK_FRONT_MASK[64];
extern const uint64_t NOT_KINGSIDE_HALF_MASK[64];
extern const uint64_t LEFT_RIGHT_COLUMN_MASK[64];
extern const uint64_t WHITE_DIAGONAL_UP_THEN_FORWARD_MASK[64];
extern const uint64_t BLACK_DIAGONAL_DOWN_THEN_FORWARD_MASK[64];
extern const uint64_t WHITE_ADJ_FILES_DOWNWARD_MASK[64];
extern const uint64_t BLACK_ADJ_FILES_UPWARD_MASK[64];
bool is_white_passed_pawn(int32_t square, uint64_t black_pawns);
bool is_black_passed_pawn(int32_t square, uint64_t white_pawns);
int32_t count_white_passed_pawns(uint64_t white_pawns, uint64_t black_pawns);
int32_t count_black_passed_pawns(uint64_t black_pawns, uint64_t white_pawns);