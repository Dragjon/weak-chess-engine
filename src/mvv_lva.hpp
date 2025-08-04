# pragma once
#include "chess.hpp"

// MVV-LVA table. We index with [victim][attacker]
constexpr int32_t mvv_lva_table[6][6]{
    {0, 0, 0, 0, 0, 0},       // victim K, attacker K, Q, R, B, N, P, None
    {500000, 510000, 520000, 530000, 540000, 550000}, // victim Q, attacker K, Q, R, B, N, P, None
    {400000, 410000, 420000, 430000, 440000, 450000}, // victim R, attacker K, Q, R, B, N, P, None
    {300000, 310000, 320000, 330000, 340000, 350000}, // victim B, attacker K, Q, R, B, N, P, None
    {200000, 210000, 220000, 230000, 240000, 250000}, // victim N, attacker K, Q, R, B, N, P, None
    {100000, 110000, 120000, 130000, 140000, 150000}, // victim P, attacker K, Q, R, B, N, P, None
};

// Function to get the value of a capture
// Returns 0 if not a capture
inline int32_t mvv_lva(chess::Board &board, chess::Move move){
    // Beware of UBs!!!!
    int32_t victim = static_cast<int32_t>(board.at(move.to()).type());
    if (victim == 6) return 0;

    int32_t attacker = static_cast<int32_t>(board.at(move.from()).type());
    if (attacker == 6) return 0;

    return mvv_lva_table[5 - victim][5 - attacker];
}