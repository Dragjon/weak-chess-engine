#include <chrono>
#include <cstdint>

// Define global variables
std::chrono::time_point<std::chrono::system_clock> search_start_time = std::chrono::system_clock::now();
int64_t max_soft_time_ms = 10000;
int64_t max_hard_time_ms = 30000;
