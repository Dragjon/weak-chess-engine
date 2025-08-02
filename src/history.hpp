#pragma once

#include <cstdint>
#include "chess.hpp"
#include "search.hpp"

// Killers
extern chess::Move killers[2][MAX_SEARCH_PLY+1];
void reset_killers();


// Quiet History [color][from][to]
constexpr int32_t MAX_HISTORY = 16384;
extern int32_t quiet_history[2][64][64];
void reset_quiet_history();


// Continuation history [previous piece][target sq][curr piece][target square]
extern int32_t one_ply_conthist[12][64][12][64];
extern int32_t two_ply_conthist[12][64][12][64];
void reset_continuation_history();

void reset_correction_history();
int32_t corrhist_adjust_eval(const chess::Board &board, int32_t raw_eval);

void update_correction_history(const chess::Board &board, int32_t depth, int32_t diff);