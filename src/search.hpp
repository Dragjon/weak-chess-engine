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
    for (int32_t i = 0; i < 2; ++i)
        for (int32_t j = 0; j < MAX_SEARCH_PLY + 1; ++j)
            killers[i][j] = chess::Move{};
}

// Quiet History [color][from][to]
constexpr int32_t MAX_HISTORY = 16384;
extern int32_t quiet_history[2][64][64];
inline void reset_quiet_history() {
    for (int32_t color = 0; color < 2; ++color) {
        for (int32_t piece = 0; piece < 64; ++piece) {
            for (int32_t square = 0; square < 64; ++square) {
                quiet_history[color][piece][square] = 0;
            }
        }
    }
}

// Continuation history [previous piece][target sq][curr piece][target square]
extern int32_t conthist[12][64][12][64];
inline void reset_continuation_history() {
    for (int32_t prev = 0; prev < 12; ++prev) {
        for (int32_t prev_sq = 0; prev_sq < 64; ++prev_sq) {
            for (int32_t curr = 0; curr < 12; ++curr) {
                for (int32_t curr_sq = 0; curr_sq < 64; ++curr_sq){
                    conthist[prev][prev_sq][curr][curr_sq] = 0;
                }
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
int32_t alpha_beta(chess::Board &board, int32_t depth, int32_t alpha, int32_t beta, int32_t ply, bool cut_node, int32_t* search_info);

// Root of the search function basically
int32_t search_root(chess::Board &board);