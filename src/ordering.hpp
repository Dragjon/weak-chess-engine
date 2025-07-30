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

void sort_moves(chess::Board& board, chess::Movelist& movelist, bool tt_hit, std::uint16_t tt_move, int32_t ply, SearchInfo search_info);
std::array<bool, 256> sort_captures(chess::Board& board, chess::Movelist& movelist, bool tt_hit, std::uint16_t tt_move);
