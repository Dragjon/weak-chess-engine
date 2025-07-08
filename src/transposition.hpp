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
    UPPERBOUND
};

// Single TT Entry
struct TTEntry {
    uint64_t key = 0; // Zobrist hash
    int32_t score = 0; // Score 
    int32_t depth = -1; // Depth
    NodeType type = NodeType::EXACT;
    uint16_t best_move = 0; // Encoded move
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

    void store(uint64_t key, int32_t score, int32_t depth, NodeType type, uint16_t bestMove) {
        size_t index = key % size;
        TTEntry& entry = table[index];

        if (entry.key == 0 || entry.depth <= depth) {
            entry = TTEntry{ key, static_cast<int32_t>(score), static_cast<int32_t>(depth), type, bestMove };
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
};

extern TranspositionTable tt;