#pragma once
#include <cstdint>
#include "chess.hpp"

extern int32_t see_piece_values[7];
bool see(chess::Board board, chess::Move move, int32_t threshold);
int32_t move_estimated_value(chess::Board &board, chess::Move move);