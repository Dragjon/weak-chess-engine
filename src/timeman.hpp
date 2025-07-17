#pragma once
#include <cstdint>
#include <chrono>
#include "defaults.hpp"
#include "search.hpp"

// Time tracking
extern int64_t max_soft_time_ms;
extern int64_t max_hard_time_ms;
extern std::chrono::time_point<std::chrono::system_clock> search_start_time;

// Get's the epased time after searching
inline int64_t elapsed_ms() {
    auto now = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - search_start_time);
    return elapsed.count();
}

// Returns true if elapsed time exceeds hard bound time limit
inline bool hard_bound_time_exceeded() {
    auto now = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - search_start_time);
    return elapsed.count() > max_hard_time_ms;
}

// returns the fraction of nodes spent on best root move compared to other moves
inline double frac_best_move_nodes(){
    return ((double)best_move_nodes)/((double)total_nodes_per_search);
}


// Totally yoinked from Potential
inline double get_bm_scale() {
    double bestMoveScale[5] = {2.43, 1.35, 1.09, 0.88, 0.68};
    return bestMoveScale[bm_stability];
}


// Returns true if elapsed time exceeds soft bound time limit
inline bool soft_bound_time_exceeded() {
    auto now = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - search_start_time);

    double prop = frac_best_move_nodes();
    double scale = ((double)(node_tm_base.current) / 100 - prop) * ((double)(node_tm_mul.current) / 100);

    // BM-Stability (https://github.com/ProgramciDusunur/Potential/commit/d1e5a2d7f03c8616abc1a2ca7779145195da3c74)
    double bm_scale = 1.0;

    if (root_best_move == previous_best_move) {
        bm_stability = std::min(bm_stability + 1, 4);
    } else {
        bm_stability = 0;
    }

    if (global_depth > 6) {
        bm_scale = get_bm_scale();
    }

    return elapsed.count() >= (int64_t)((double)max_soft_time_ms * scale * bm_scale);
}