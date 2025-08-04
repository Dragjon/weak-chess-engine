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

// Main search sorting
void sort_moves(Board& board, Movelist& movelist, bool tt_hit, uint16_t tt_move, int32_t ply, SearchInfo search_info) {

    int32_t parent_move_piece = search_info.parent_move_piece;
    int32_t parent_move_square = search_info.parent_move_square;
    int32_t parent_parent_move_piece = search_info.parent_parent_move_piece;
    int32_t parent_parent_move_square = search_info.parent_parent_move_square;

    const size_t move_count = movelist.size();
    assert(move_count <= MAX_MOVES);

    std::array<std::pair<int32_t, Move>, MAX_MOVES> scored_moves;

    for (size_t i = 0; i < move_count; i++) {
        const auto& move = movelist[i];
        int32_t to = move.to().index();
        int32_t move_piece = static_cast<int32_t>(board.at(move.from()).internal());
        int32_t captured = static_cast<int32_t>(board.at(move.to()).internal());

        int32_t score = 0;

        if (tt_hit && move.move() == tt_move) {
            score = TT_BONUS;
        } else if (board.isCapture(move)) {
            score = mvv(board, move) + capthist[move_piece][to][captured];
            score += see(board, move, 0) ? 0 : -10000000;
        } else if (killers[0][ply] == move || killers[1][ply] == move) {
            score = KILLER_BONUS;
        } else {
            score = quiet_history[board.sideToMove() == Color::WHITE][move.from().index()][move.to().index()];

            if (parent_move_piece != -1 && parent_move_square != -1)
                score += one_ply_conthist[parent_move_piece][parent_move_square][static_cast<int32_t>(board.at(move.from()).internal())][move.to().index()];

            if (parent_parent_move_piece != -1 && parent_parent_move_square != -1)
                score += two_ply_conthist[parent_parent_move_piece][parent_parent_move_square][static_cast<int32_t>(board.at(move.from()).internal())][move.to().index()];
        }

        scored_moves[i] = std::make_pair(score, move);
    }

    std::stable_sort(scored_moves.begin(), scored_moves.begin() + move_count, [](const auto& a, const auto& b) {
        return a.first > b.first;
    });

    for (size_t i = 0; i < move_count; i++) {
        movelist[i] = scored_moves[i].second;
    }
}

// Captures sorting — returns a fixed-size std::array<bool, MAX_MOVES> and length
std::array<bool, MAX_MOVES> sort_captures(Board& board, Movelist& movelist, bool tt_hit, std::uint16_t tt_move) {
    const size_t move_count = movelist.size();
    assert(move_count <= MAX_MOVES);

    std::array<std::tuple<int32_t, bool, Move>, MAX_MOVES> scored_moves;
    std::array<bool, MAX_MOVES> result_sees;

    for (size_t i = 0; i < move_count; i++) {
        const auto& move = movelist[i];
        int32_t to = move.to().index();
        int32_t move_piece = static_cast<int32_t>(board.at(move.from()).internal());
        int32_t captured = static_cast<int32_t>(board.at(move.to()).internal());

        bool good_see = see(board, move, 0);
        int32_t score = tt_hit && move.move() == tt_move
                        ? TT_BONUS
                        : mvv(board, move) + (good_see ? 0 : -10000000) + capthist[move_piece][to][captured];

        scored_moves[i] = std::make_tuple(score, good_see, move);
    }

    std::stable_sort(scored_moves.begin(), scored_moves.begin() + move_count, [](const auto& a, const auto& b) {
        return std::get<0>(a) > std::get<0>(b);
    });

    for (size_t i = 0; i < move_count; i++) {
        const auto& [score, see_val, move] = scored_moves[i];
        movelist[i] = move;
        result_sees[i] = see_val;
    }

    return result_sees;
}
