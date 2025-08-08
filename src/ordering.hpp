#pragma once

#include <array>
#include <cstdint>
#include <utility>
#include <cassert>

#include "chess.hpp"
#include "ordering.hpp"
#include "transposition.hpp"
#include "mvv_lva.hpp"
#include "see.hpp"
#include "search_info.hpp"

void sort_moves_lazy(chess::Board& board, chess::Movelist& movelist, bool tt_hit, uint16_t tt_move, int32_t ply, SearchInfo search_info);
void sort_captures_lazy(chess::Board& board, chess::Movelist& movelist, bool tt_hit, uint16_t tt_move, int32_t ply);
chess::Move get_next_move(int32_t ply);
chess::Move get_next_capture(int32_t ply);