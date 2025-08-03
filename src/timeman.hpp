#pragma once
#include <cstdint>
#include <chrono>
#include "defaults.hpp"
#include "search.hpp"

// Time tracking
extern int64_t max_soft_time_ms;
extern int64_t max_hard_time_ms;
extern int64_t move_overhead_ms;
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
extern double best_move_scale[5];
inline double get_bm_scale() {
    return best_move_scale[bm_stability];
}

// Inspired by Potential's eval stability
extern double score_scale[5];
inline double get_score_scale() {
    return score_scale[score_stability];
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


    // Score stability also yoinked from Potential (https://github.com/ProgramciDusunur/Potential/pull/220/commits/ea410b0666d38ae05b8c66d67bc45358f35a17b8)
    double score_scale = 1.0;
    if (root_best_score > avg_prev_score - 10 && root_best_score < avg_prev_score + 10){
        score_stability = std::min(score_stability + 1, 4);
    }
    else {
        score_stability = 0;
    }

    if (global_depth >= 7) {
        bm_scale = get_bm_scale();
        score_scale = get_score_scale();
    }
    
    return elapsed.count() >= (int64_t)((double)max_soft_time_ms * scale * bm_scale * score_scale);
}