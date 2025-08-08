#include <array>
#include <cstdint>
#include <utility>
#include <cassert>
#include <algorithm>

#include "chess.hpp"
#include "ordering.hpp"
#include "transposition.hpp"
#include "mvv_lva.hpp"
#include "see.hpp"
#include "search_info.hpp"
#include "history.hpp"

using namespace chess;

constexpr int32_t TT_BONUS = 1000000;
constexpr int32_t KILLER_BONUS = 90000;
constexpr size_t MAX_MOVES = 256;
constexpr size_t MAX_PLY   = 255;

/****************************************
*             SEARCH SORTING            *
*****************************************/

// Now per-ply storage
static std::array<std::array<std::pair<int32_t, Move>, MAX_MOVES>, MAX_PLY+1> g_scored_moves;
static std::array<size_t, MAX_PLY+1> g_move_count{};
static std::array<std::array<bool, 65536>, MAX_PLY+1> g_move_used{};

// Score moves without sorting — sets up per-ply scored move list
void sort_moves_lazy(Board& board, Movelist& movelist, bool tt_hit, uint16_t tt_move, int32_t ply, SearchInfo search_info)
{
    // Clear used flags for this ply only
    std::fill(std::begin(g_move_used[ply]), std::end(g_move_used[ply]), false);

    g_move_count[ply] = movelist.size();
    assert(g_move_count[ply] <= MAX_MOVES);

    int32_t parent_move_piece = search_info.parent_move_piece;
    int32_t parent_move_square = search_info.parent_move_square;
    int32_t parent_parent_move_piece = search_info.parent_parent_move_piece;
    int32_t parent_parent_move_square = search_info.parent_parent_move_square;

    for (size_t i = 0; i < g_move_count[ply]; i++) {
        const auto& move = movelist[i];
        int32_t score = 0;

        if (tt_hit && move.move() == tt_move) {
            score = TT_BONUS;
        } else if (board.isCapture(move)) {
            score = mvv_lva(board, move);
            score += see(board, move, 0) ? 0 : -10000000;
        } else if (killers[0][ply] == move || killers[1][ply] == move) {
            score = KILLER_BONUS;
        } else {
            score = quiet_history[board.sideToMove() == Color::WHITE][move.from().index()][move.to().index()];

            if (parent_move_piece != -1 && parent_move_square != -1)
                score += one_ply_conthist[parent_move_piece][parent_move_square]
                         [static_cast<int32_t>(board.at(move.from()).internal())][move.to().index()];

            if (parent_parent_move_piece != -1 && parent_parent_move_square != -1)
                score += two_ply_conthist[parent_parent_move_piece][parent_parent_move_square]
                         [static_cast<int32_t>(board.at(move.from()).internal())][move.to().index()];
        }

        g_scored_moves[ply][i] = std::make_pair(score, move);
    }
}

// Get next highest scoring unused move for a ply
Move get_next_move(int32_t ply)
{
    int32_t best_score = -1000000000;
    size_t best_index = SIZE_MAX;

    for (size_t i = 0; i < g_move_count[ply]; i++) {
        uint16_t move_id = g_scored_moves[ply][i].second.move();

        if (!g_move_used[ply][move_id] && g_scored_moves[ply][i].first > best_score) {
            best_score = g_scored_moves[ply][i].first;
            best_index = i;
        }
    }

    if (best_index != SIZE_MAX) {
        uint16_t move_id = g_scored_moves[ply][best_index].second.move();
        g_move_used[ply][move_id] = true;
        return g_scored_moves[ply][best_index].second;
    }

    // No moves left for this ply
    g_move_count[ply] = 0;
    return Move();
}


/****************************************
*           CAPTURES SORTING            *
*****************************************/

// Per-ply storage for captures
static std::array<std::array<std::pair<int32_t, Move>, MAX_MOVES>, MAX_PLY+1> g_scored_captures;
static std::array<size_t, MAX_PLY+1> g_capture_count{};
static std::array<std::array<bool, 65536>, MAX_PLY+1> g_capture_used{};

// Score captures without sorting
void sort_captures_lazy(Board& board, Movelist& movelist, bool tt_hit, uint16_t tt_move, int32_t ply)
{
    // Reset used flags for this ply
    std::fill(std::begin(g_capture_used[ply]), std::end(g_capture_used[ply]), false);

    g_capture_count[ply] = movelist.size();
    assert(g_capture_count[ply] <= MAX_MOVES);

    for (size_t i = 0; i < g_capture_count[ply]; i++) {
        const auto& move = movelist[i];

        int32_t score = (tt_hit && move.move() == tt_move)
                        ? TT_BONUS
                        : mvv_lva(board, move) + (see(board, move, 0) ? 0 : -10000000);

        g_scored_captures[ply][i] = std::make_pair(score, move);
    }
}

// Get next best unused capture for a ply
Move get_next_capture(int32_t ply)
{
    int32_t best_score = -1000000000;
    size_t best_index = SIZE_MAX;

    for (size_t i = 0; i < g_capture_count[ply]; i++) {
        uint16_t move_id = g_scored_captures[ply][i].second.move();

        if (!g_capture_used[ply][move_id] && g_scored_captures[ply][i].first > best_score) {
            best_score = g_scored_captures[ply][i].first;
            best_index = i;
        }
    }

    if (best_index != SIZE_MAX) {
        uint16_t move_id = g_scored_captures[ply][best_index].second.move();
        g_capture_used[ply][move_id] = true;
        return g_scored_captures[ply][best_index].second;
    }

    // No captures left
    g_capture_count[ply] = 0;
    return Move();
}
