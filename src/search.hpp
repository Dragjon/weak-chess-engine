#pragma once
#include <stdexcept>
#include <stdint.h>

#include "chess.hpp"
#include "search_info.hpp"

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
extern chess::Move previous_best_move;

// BM-Stability time management (https://github.com/ProgramciDusunur/Potential/commit/d1e5a2d7f03c8616abc1a2ca7779145195da3c74)
extern int32_t bm_stability;

// Eval stability time management (https://github.com/ProgramciDusunur/Potential/pull/220/commits/ea410b0666d38ae05b8c66d67bc45358f35a17b8)
extern int32_t score_stability;
extern int32_t root_best_score;
extern int32_t avg_prev_score;

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
int32_t alpha_beta(chess::Board &board, int32_t depth, int32_t alpha, int32_t beta, int32_t ply, int32_t branch, bool cut_node, SearchInfo search_info);

// Root of the search function basically
int32_t search_root(chess::Board &board);
