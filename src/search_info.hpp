#pragma once

#include <cstdint>

struct SearchInfo {
    int32_t parent_move_piece = -1;
    int32_t parent_move_square = -1;
    int32_t parent_parent_move_piece = -1;
    int32_t parent_parent_move_square = -1;
    uint16_t excluded = 0;
    int32_t nmp_min_ply = 0;
    // int32_t parent_static_eval = -100000;
    // int32_t parent_parent_eval = -100000;

    SearchInfo() = default;
};
