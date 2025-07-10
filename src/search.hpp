#pragma once
#include <stdexcept>
#include <stdint.h>

#include "chess.hpp"

// For mate scoring and default value form max_score
constexpr int32_t POSITIVE_MATE_SCORE = 50000;
constexpr int32_t POSITIVE_WIN_SCORE = 45000;
constexpr int32_t POSITIVE_INFINITY = 100000;
constexpr int32_t DEFAULT_ALPHA = -POSITIVE_INFINITY;
constexpr int32_t DEFAULT_BETA = POSITIVE_INFINITY;

// Maximum search depth
constexpr int32_t MAX_SEARCH_DEPTH = 128;
constexpr int32_t MAX_SEARCH_PLY = 255;

// Our custom error
struct SearchAbort : public std::exception {
    const char* what() const noexcept override {
        return "Abort Search!!!! Ahhh!!";
    }
};

// The global best move variable
extern chess::Move root_best_move;

// Killers
extern chess::Move killers[2][MAX_SEARCH_PLY+1];
inline void reset_killers(){
    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < MAX_SEARCH_PLY + 1; ++j)
            killers[i][j] = chess::Move{};
}


// Maximum history score for both capthist and quiet hist
constexpr int32_t MAX_HISTORY = 16384;

// Quiet History [color][from][to]
extern int32_t quiet_history[2][64][64];
inline void reset_quiet_history() {
    for (int color = 0; color < 2; ++color) {
        for (int from = 0; from < 64; ++from) {
            for (int square = 0; square < 64; ++square) {
                quiet_history[color][from][square] = 0;
            }
        }
    }
}


// Capture History [piece][target][captured]
// size 7 for 3rd dimension to be safer and avoid ub
extern int32_t capture_history[6][64][7];
inline void reset_capture_history() {
    for (int piece = 0; piece < 6; ++piece) {
        for (int target = 0; target < 64; ++piece) {
            for (int captured = 0; captured < 64; ++captured) {
                quiet_history[piece][target][captured] = 0;
            }
        }
    }
}

// The global depth variable
extern int32_t global_depth;

extern int64_t best_move_nodes;
extern int64_t total_nodes_per_search;
extern int64_t total_nodes;

extern int32_t seldpeth;

// Search Function
// We are basically using a fail soft "negamax" search, see here for more info: https://minuskelvin.net/chesswiki/content/minimax.html#negamax
// Negamax is basically a simplification of the famed minimax algorithm. Basically, it works by negating the score in the next
// ply. This works because a position which is a win for white is a loss for black and vice versa. Most "strong" chess engines use
// negamax instead of minimax because it makes the code much tidier. Not sure about how much is gains though. The "fail soft" basically
// means we return max_value instead of alpha. This gives us more information to do puning etc etc.
int32_t alpha_beta(chess::Board &board, int32_t depth, int32_t alpha, int32_t beta, int32_t ply, bool cut_node);

// Root of the search function basically
int32_t search_root(chess::Board &board);