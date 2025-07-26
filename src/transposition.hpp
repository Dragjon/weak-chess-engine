#pragma once
#include <cstdint>
#include <vector>
#include <limits>

#include "chess.hpp"

const int32_t TT_DEFAULT_SIZE = 64;

// TT Node Types
enum class NodeType : uint8_t {
    EXACT,
    LOWERBOUND,
    UPPERBOUND,
    NONE
};

// Single TT Entry
struct TTEntry {
    uint64_t key = 0; // Zobrist hash
    int32_t score = 0; // Score 
    int32_t depth = -1; // Depth
    NodeType type = NodeType::NONE;
    uint16_t best_move = 0; // Encoded move
    bool tt_was_pv = false;
};

// Transposition table class
class TranspositionTable {
    std::vector<TTEntry> table;
    size_t size;

public:
    TranspositionTable(size_t mb = 64) {
        size = (mb * 1024 * 1024) / sizeof(TTEntry);
        table.resize(size);
    }

    void clear() {
        std::fill(table.begin(), table.end(), TTEntry{});
    }

    void resize(size_t mb) {
        size = (mb * 1024 * 1024) / sizeof(TTEntry);
        table.clear();
        table.resize(size);
    }

    void store(uint64_t key, int32_t score, int32_t depth, NodeType type, uint16_t best_move, bool tt_was_pv) {
        size_t index = key % size;
        TTEntry& entry = table[index];

        if (entry.key == 0 || entry.depth <= depth) {
            entry = TTEntry{ key, score, depth, type, best_move, tt_was_pv };
        }
    }

    bool probe(uint64_t key, TTEntry& out) const {
        const TTEntry& entry = table[key % size];
        if (entry.key == key) {
            out = entry;
            return true;
        }
        return false;
    }

    int32_t hashfull() const {
        int32_t fill = 0;
        size_t probe_limit = std::min(size, size_t(1000));

        for (size_t i = 0; i < probe_limit; ++i) {
            if (table[i].type != NodeType::NONE) {
                ++fill;
            }
        }
        return fill;
    }
};

extern TranspositionTable tt;