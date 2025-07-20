#include <iostream>
#include <random>
#include <cstdint>
#include <iomanip>
#include <string>

const char* PIECE_NAMES[6] = {
    "PAWN", "KNIGHT", "BISHOP", "ROOK", "QUEEN", "KING"
};

int main() {
    uint64_t randoms[6][2][64]; // [piece][color][square]

    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;

    // Generate random numbers
    for (int piece = 0; piece < 6; ++piece)
        for (int color = 0; color < 2; ++color)
            for (int square = 0; square < 64; ++square)
                randoms[piece][color][square] = dis(gen);

    // Output each piece array
    for (int piece = 0; piece < 6; ++piece) {
        std::cout << "const uint64_t " << PIECE_NAMES[piece] << "_RANDOMS[2][64] = {\n";
        for (int color = 0; color < 2; ++color) {
            std::cout << "    { ";
            for (int square = 0; square < 64; ++square) {
                std::cout << "0x" << std::hex << std::setw(16) << std::setfill('0') << randoms[piece][color][square];
                if (square != 63) std::cout << ", ";
            }
            std::cout << " }";
            if (color != 1) std::cout << ",";
            std::cout << "\n";
        }
        std::cout << "};\n\n";
    }

    return 0;
}
