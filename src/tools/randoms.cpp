#include <iostream>
#include <random>
#include <cstdint>
#include <iomanip>

int main() {
    uint64_t arr[2][64];

    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;

    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 64; ++j)
            arr[i][j] = dis(gen);

    std::cout << "const uint64_t PAWN_RANDOMS[2][64] = {\n";
    for (int i = 0; i < 2; ++i) {
        std::cout << "    { ";
        for (int j = 0; j < 64; ++j) {
            std::cout << "0x" << std::hex << std::setw(16) << std::setfill('0') << arr[i][j];
            if (j != 63) std::cout << ", ";
        }
        std::cout << " }";
        if (i != 1) std::cout << ",";
        std::cout << "\n";
    }
    std::cout << "};\n";

    return 0;
}
