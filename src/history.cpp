#include <cstdint>
#include "chess.hpp"
#include "search.hpp"
#include "history.hpp"
#include "bitboard.hpp"

using namespace chess;
using namespace std;

// Histories
Move killers[2][MAX_SEARCH_PLY+1]{};
int32_t quiet_history[2][64][64]{};
int32_t one_ply_conthist[12][64][12][64]{};
int32_t two_ply_conthist[12][64][12][64]{};

// Correction history :-)
// [0] -> white, [1] -> black for consistency
int32_t pawn_correction_history[2][16384]{};

// Reset killer moves
void reset_killers(){
    for (int32_t i = 0; i < 2; ++i)
        for (int32_t j = 0; j < MAX_SEARCH_PLY + 1; ++j)
            killers[i][j] = chess::Move{};
}


// Reset quiet histiry
void reset_quiet_history() {
    for (int32_t color = 0; color < 2; ++color) {
        for (int32_t piece = 0; piece < 64; ++piece) {
            for (int32_t square = 0; square < 64; ++square) {
                quiet_history[color][piece][square] = 0;
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

// Reset correction histories
void reset_correction_history() {
    for (int32_t color = 0; color < 2; ++color) {
        for (int32_t hashkey = 0; hashkey < 16384; ++hashkey){
            pawn_correction_history[color][hashkey] = 0;
        }
    }
}

// Updates the pawn correction history given the difference between static eval and score
// Reference: https://github.com/ProgramciDusunur/Potential/pull/221/commits/ea7701117ca87c9fffaf05330ee7029093150520
void update_pawn_correction_history(const Board &board, int32_t depth, int32_t diff) {
    uint64_t pawn_key = get_pawn_hash(board);
    int32_t entry = pawn_correction_history[board.sideToMove() == Color::WHITE ? 0 : 1][pawn_key % 16384];
    int32_t scaled_diff = diff * 256;
    int32_t newWeight = min(depth + 1, 16);
    entry = (entry * (256 - newWeight) + scaled_diff * newWeight) / 256;
    entry = clamp(entry, -16384, 16384);
}

// Function to use correction history to adjust static eval
int32_t corrhist_adjust_eval(const Board &board, int32_t raw_eval) {
    uint64_t pawn_key = get_pawn_hash(board);
    int32_t entry = pawn_correction_history[board.sideToMove() == Color::WHITE ? 0 : 1][pawn_key % 16384];
    return clamp(raw_eval + entry / 256, -40000, 40000);
}