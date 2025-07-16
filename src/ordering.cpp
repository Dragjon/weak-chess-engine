#include <array>
#include <cstdint>
#include <utility>
#include <cassert>
#include <vector>
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

// Main search sorting
void sort_moves(Board& board, Movelist& movelist, bool tt_hit, uint16_t tt_move, int32_t ply, SearchInfo search_info) {

    int32_t parent_move_piece = search_info.parent_move_piece;
    int32_t parent_move_square = search_info.parent_move_square;
    int32_t parent_parent_move_piece = search_info.parent_parent_move_piece;
    int32_t parent_parent_move_square = search_info.parent_parent_move_square;

    const size_t move_count = movelist.size();
    assert(move_count <= 256); 

    std::vector<std::pair<int32_t, Move>> scored_moves;
    scored_moves.reserve(move_count);

    for (size_t i = 0; i < move_count; i++) {
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

            if (parent_move_piece != -1 && parent_move_square != -1){
                score += one_ply_conthist[parent_move_piece][parent_move_square][static_cast<int32_t>(board.at(move.from()).internal())][move.to().index()];
            }

            if (parent_parent_move_piece != -1 && parent_parent_move_square != -1){
                score += two_ply_conthist[parent_parent_move_piece][parent_parent_move_square][static_cast<int32_t>(board.at(move.from()).internal())][move.to().index()];
            }
        }

        scored_moves.emplace_back(score, move);
    }

    std::stable_sort(scored_moves.begin(), scored_moves.end(), [](const auto& a, const auto& b) {
        return a.first > b.first;
    });

    for (size_t i = 0; i < move_count; i++) {
        movelist[i] = scored_moves[i].second;
    }
}


// Captures sorting
// returns a boolean vector matching the see bool result of each capture
std::vector<bool> sort_captures(Board& board, Movelist& movelist, bool tt_hit, std::uint16_t tt_move) {
    const size_t move_count = movelist.size();
    assert(move_count <= 256);

    // Score + see + move tuple for sorting
    std::vector<std::tuple<int32_t, bool, Move>> scored_moves;
    scored_moves.reserve(move_count);

    for (size_t i = 0; i < move_count; i++) {
        const auto& move = movelist[i];
        int32_t score = 0;
        bool good_see = see(board, move, 0);

        if (tt_hit && move.move() == tt_move) {
            score = TT_BONUS;
        } else {
            score = mvv_lva(board, move);
            score += good_see ? 0 : -10000000;
        }

        scored_moves.emplace_back(score, good_see, move);
    }

    // Sort descending by score
    std::stable_sort(scored_moves.begin(), scored_moves.end(), [](const auto& a, const auto& b) {
        return std::get<0>(a) > std::get<0>(b); // sort by score descending
    });

    // Write back to movelist and sees
    std::vector<bool> result_sees;
    result_sees.reserve(move_count);

    for (size_t i = 0; i < move_count; i++) {
        const auto& [score, see_val, move] = scored_moves[i];
        movelist[i] = move;
        result_sees.push_back(see_val);
    }

    return result_sees;
}
