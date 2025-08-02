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
int32_t non_pawn_correction_history[2][16384]{};
int32_t minor_correction_history[2][16384]{};
int32_t major_correction_history[2][16384]{};
int32_t threat_history[2][16384]{};

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
        for (int32_t hash_key = 0; hash_key < 16384; ++hash_key){
            pawn_correction_history[color][hash_key] = 0;
            non_pawn_correction_history[color][hash_key] = 0;
            minor_correction_history[color][hash_key] = 0;
            major_correction_history[color][hash_key] = 0;
            threat_history[color][hash_key] = 0;
        }
    }
}

const int32_t MAX_CORRHIST = 1024;

// Updates all the correction histories given the difference between static eval and score
// Reference: https://github.com/ProgramciDusunur/Potential/pull/221/commits/ea7701117ca87c9fffaf05330ee7029093150520
// Another reference: https://github.com/Bobingstern/Tarnished/blob/master/src/search.h#L277
void update_correction_history(const Board &board, int32_t depth, int32_t diff) {
    uint64_t pawn_key = get_pawn_key(board);
    uint64_t non_pawn_key = get_non_pawn_key(board);
    uint64_t minors_key = get_minors_key(board);
    uint64_t majors_key = get_majors_key(board);
    uint64_t threat_key = get_threat_key(board);
    
    int32_t pawn_key_idx = pawn_key % 16384;
    int32_t non_pawn_key_idx = non_pawn_key % 16384;
    int32_t minors_key_idx = minors_key % 16384;
    int32_t majors_key_idx = majors_key % 16384;
    int32_t threat_key_idx = threat_key % 16384;

    int32_t stm = board.sideToMove() == Color::WHITE ? 0 : 1;
    int32_t clamped_diff = clamp(diff, -MAX_CORRHIST / 4, MAX_CORRHIST / 4);

    // History gravity formula for corrhist
    pawn_correction_history[stm][pawn_key_idx] += clamped_diff - pawn_correction_history[stm][pawn_key_idx] * abs(clamped_diff) / MAX_CORRHIST;
    non_pawn_correction_history[stm][non_pawn_key_idx] += clamped_diff - non_pawn_correction_history[stm][non_pawn_key_idx] * abs(clamped_diff) / MAX_CORRHIST;
    minor_correction_history[stm][minors_key_idx] += clamped_diff - minor_correction_history[stm][minors_key_idx] * abs(clamped_diff) / MAX_CORRHIST;
    major_correction_history[stm][majors_key_idx] += clamped_diff - major_correction_history[stm][majors_key_idx] * abs(clamped_diff) / MAX_CORRHIST;
    threat_history[stm][threat_key_idx] += clamped_diff - threat_history[stm][threat_key_idx] * abs(clamped_diff) / MAX_CORRHIST;
}

// Function to use correction history to adjust static eval
// Original weights were roughly based on Tarnished (https://github.com/Bobingstern/Tarnished/blob/master/src/search.h#L331)
int32_t corrhist_adjust_eval(const Board &board, int32_t raw_eval) {
    uint64_t pawn_key = get_pawn_key(board);
    uint64_t non_pawn_key = get_non_pawn_key(board);
    uint64_t minors_key = get_minors_key(board);
    uint64_t majors_key = get_majors_key(board);
    uint64_t threat_key = get_threat_key(board);

    int32_t pawn_key_idx = pawn_key % 16384;
    int32_t non_pawn_key_idx = non_pawn_key % 16384;
    int32_t minors_key_idx = minors_key % 16384;
    int32_t majors_key_idx = majors_key % 16384;
    int32_t threat_key_idx = threat_key % 16384;

    int32_t stm = board.sideToMove() == Color::WHITE ? 0 : 1;
    int32_t correction = 200 * pawn_correction_history[stm][pawn_key_idx] + 160 * non_pawn_correction_history[stm][non_pawn_key_idx] + 150 * minor_correction_history[stm][minors_key_idx] + 140 * major_correction_history[stm][majors_key_idx] + 150 * threat_history[stm][threat_key_idx];

    return clamp(raw_eval + correction / 2048, -40000, 40000);
}