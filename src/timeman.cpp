#include <chrono>
#include <cstdint>

// Define global variables
std::chrono::time_point<std::chrono::system_clock> search_start_time = std::chrono::system_clock::now();
int64_t max_soft_time_ms = 10000ll;
int64_t max_hard_time_ms = 30000ll;
int64_t move_overhead_ms = 0; 

double best_move_scale[5] = {2.37, 1.32, 1.18, 0.83, 0.53};
double score_scale[5] = {1.21, 1.09, 1.00, 0.87, 0.99};