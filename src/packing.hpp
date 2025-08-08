#pragma once
#include <cstdint>

// Packs two 16-bit signed scores (midgame, endgame) into a single 32-bit integer.
inline constexpr int32_t S(int16_t mg, int16_t eg) noexcept {
    return (static_cast<uint32_t>(static_cast<uint16_t>(eg)) << 16) | static_cast<uint16_t>(mg);
}

// Extracts the midgame component (lower 16 bits).
inline constexpr int16_t unpack_mg(int32_t packed) noexcept {
    return static_cast<int16_t>(packed & 0xFFFF);
}

// Extracts the endgame component (upper 16 bits).
inline constexpr int16_t unpack_eg(int32_t packed) noexcept {
    return static_cast<int16_t>((packed >> 16) & 0xFFFF);
}
