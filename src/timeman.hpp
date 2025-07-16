#pragma once
#include <cstdint>
#include <chrono>
#include <cmath>

#include "defaults.hpp"
#include "search.hpp"
#include "algorithm"
#include "eval.hpp"

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

// Returns true if elapsed time exceeds soft bound time limit
inline bool soft_bound_time_exceeded(const chess::Board &board) {
    auto now = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - search_start_time);

    double prop = frac_best_move_nodes();
    double scale = ((double)(node_tm_base.current) / 100 - prop) * ((double)(node_tm_mul.current) / 100);

    double complexity = 0.8 * log(global_depth) * abs(evaluate(board) - root_score);
    double scale1 = (0.7 + std::clamp(complexity, 0.0, 200.0) / 400.0);

    return elapsed.count() >= (int64_t)((double)max_soft_time_ms * scale * scale1);
}