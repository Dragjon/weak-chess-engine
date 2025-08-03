#include <chrono>
#include <cstdint>

// Define global variables
std::chrono::time_point<std::chrono::system_clock> search_start_time = std::chrono::system_clock::now();
int64_t max_soft_time_ms = 10000ll;
int64_t max_hard_time_ms = 30000ll;
int64_t move_overhead_ms = 0; 

double best_move_scale[5] = {2.43, 1.35, 1.09, 0.88, 0.68};
double score_scale[5] = {1.25, 1.15, 1.00, 0.94, 0.88};