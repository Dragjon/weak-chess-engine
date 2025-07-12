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

constexpr int32_t TT_BONUS = 1000000;
constexpr int32_t KILLER_BONUS = 90000;

inline void sort_moves(chess::Board& board, chess::Movelist& movelist, bool tt_hit, uint16_t tt_move, int32_t ply, int32_t* search_info) {

    int32_t parent_move_piece = search_info[0];
    int32_t parent_move_square = search_info[1];
    int32_t parent_parent_move_piece = search_info[2];
    int32_t parent_parent_move_square = search_info[3];


    const size_t move_count = movelist.size();
    assert(move_count <= 256); 

    // Score array for corresponding moves
    std::array<int32_t, 256> scores;

    for (size_t i = 0; i < move_count; i++) {
        const auto& move = movelist[i];
        int32_t score = 0;

        if (tt_hit && move.move() == tt_move) {
            score = TT_BONUS;
        } else if (board.isCapture(move)) {
            score = mvv_lva(board, move);
        } else if (killers[0][ply] == move || killers[1][ply] == move) {
            score = KILLER_BONUS;
        } else {
            score = quiet_history[board.sideToMove() == chess::Color::WHITE][move.from().index()][move.to().index()];

            // Countermoves
            if (parent_move_piece != 99 && parent_move_square != 99)
                score += conthist[parent_move_piece][parent_move_square][static_cast<int32_t>(board.at(move.from()).internal())][move.to().index()];

            // Follow-up moves
            if (parent_parent_move_piece != 99 && parent_parent_move_square != 99)
                score += conthist[parent_parent_move_piece][parent_parent_move_square][static_cast<int32_t>(board.at(move.from()).internal())][move.to().index()];

        }

        scores[i] = score;
    }

    // In-place sort (descending order) using scores
    for (size_t i = 0; i < move_count; i++) {
        size_t max_idx = i;
        for (size_t j = i + 1; j < move_count; j++) {
            if (scores[j] > scores[max_idx]) {
                max_idx = j;
            }
        }
        if (max_idx != i) {
            std::swap(scores[i], scores[max_idx]);
            std::swap(movelist[i], movelist[max_idx]);
        }
    }
}

inline void sort_captures(chess::Board& board, chess::Movelist& movelist, bool tt_hit, uint16_t tt_move) {
    const size_t move_count = movelist.size();
    assert(move_count <= 256);

    std::array<int32_t, 256> scores;

    for (size_t i = 0; i < move_count; i++) {
        const auto& move = movelist[i];
        int32_t score = 0;

        if (tt_hit && move.move() == tt_move) {
            score = TT_BONUS;
        } else {
            score = mvv_lva(board, move);
        }

        scores[i] = score;
    }

    // In-place selection sort
    for (size_t i = 0; i < move_count; i++) {
        size_t max_idx = i;
        for (size_t j = i + 1; j < move_count; j++) {
            if (scores[j] > scores[max_idx]) {
                max_idx = j;
            }
        }
        if (max_idx != i) {
            std::swap(scores[i], scores[max_idx]);
            std::swap(movelist[i], movelist[max_idx]);
        }
    }
}
