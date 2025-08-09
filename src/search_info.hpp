#pragma once

#include <cstdint>

struct SearchInfo {
    int32_t parent_move_piece = -1;
    int32_t parent_move_square = -1;
    int32_t parent_parent_move_piece = -1;
    int32_t parent_parent_move_square = -1;
    uint16_t excluded = 0;

    SearchInfo() = default;
};
