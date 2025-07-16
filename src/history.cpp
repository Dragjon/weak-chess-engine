#include <cstdint>
#include "chess.hpp"
#include "search.hpp"
#include "history.hpp"

using namespace chess;

/////////////////////
///// Histories /////
////////////////////

// color-ply
Move killers[2][MAX_SEARCH_PLY+1]{};
// color-from-to
int32_t quiet_history[2][64][64]{};
// parent-parentsq-current-currentsq
int32_t one_ply_conthist[12][64][12][64]{};
// parentparent, parentparentsq, current, currentsq
int32_t two_ply_conthist[12][64][12][64]{};
// piece-to-captured
int32_t capture_hist[12][64][12]{};

// Reset killer moves
void reset_killers(){
    for (int32_t i = 0; i < 2; ++i)
        for (int32_t j = 0; j < MAX_SEARCH_PLY + 1; ++j)
            killers[i][j] = chess::Move{};
}


// Reset quiet history
void reset_quiet_history() {
    for (int32_t color = 0; color < 2; ++color) {
        for (int32_t piece = 0; piece < 64; ++piece) {
            for (int32_t square = 0; square < 64; ++square) {
                quiet_history[color][piece][square] = 0;
            }
        }
    }
}


// Reset capture history
void reset_capture_history() {
    for (int32_t piece = 0; piece < 12; ++piece) {
        for (int32_t to = 0; to < 64; ++to) {
            for (int32_t captured = 0; captured < 12; ++captured) {
                capture_hist[piece][to][captured] = 0;
            }
        }
    }
}


// Reset continuation history
void reset_continuation_history() {
    for (int32_t prev = 0; prev < 12; ++prev) {
        for (int32_t prev_sq = 0; prev_sq < 64; ++prev_sq) {
            for (int32_t curr = 0; curr < 12; ++curr) {
                for (int32_t curr_sq = 0; curr_sq < 64; ++curr_sq){
                    one_ply_conthist[prev][prev_sq][curr][curr_sq] = 0;
                    two_ply_conthist[prev][prev_sq][curr][curr_sq] = 0;
                }
            }
        }
    }
}