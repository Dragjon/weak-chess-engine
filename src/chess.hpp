/*
MIT License

Copyright (c) 2023 disservin

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

THIS FILE IS AUTO GENERATED DO NOT CHANGE MANUALLY.

Source: https://github.com/Disservin/chess-library

VERSION: 0.8.2
*/

#ifndef CHESS_HPP
#define CHESS_HPP


#include <functional>
#include <utility>


#include <cstdint>
#ifdef CHESS_USE_PEXT
#    include <immintrin.h>
#endif


#if __cpp_lib_bitops >= 201907L
#    include <bit>
#endif
#include <algorithm>
#include <bitset>
#include <cassert>
#include <iostream>
#include <string>

#if defined(_MSC_VER)
#    include <intrin.h>
#    include <nmmintrin.h>
#endif


#include <string_view>


#include <ostream>

namespace chess {

class Color {
   public:
    enum class underlying : std::int8_t { WHITE = 0, BLACK = 1, NONE = -1 };

    constexpr Color() : color(underlying::NONE) {}
    constexpr Color(underlying c) : color(c) { assert(isValid(static_cast<int>(c))); }
    constexpr Color(int c) : Color(static_cast<underlying>(c)) { assert(isValid(c)); }
    constexpr Color(std::string_view str)
        : color(str == "w"   ? underlying::WHITE
                : str == "b" ? underlying::BLACK
                             : underlying::NONE) {}

    /**
     * @brief Gets the long string representation of the color
     * @return "White" for WHITE, "Black" for BLACK
     * "None" for NONE
     */
    [[nodiscard]] std::string longStr() const {
        switch (color) {
            case underlying::WHITE:
                return "White";
            case underlying::BLACK:
                return "Black";
            default:
                return "None";
        }
    }

    constexpr Color operator~() const noexcept {
        assert(color != underlying::NONE);
        return Color(static_cast<underlying>(static_cast<int>(color) ^ 1));
    }

    constexpr bool operator==(const Color& rhs) const noexcept { return color == rhs.color; }
    constexpr bool operator!=(const Color& rhs) const noexcept { return color != rhs.color; }

    constexpr operator int() const noexcept { return static_cast<int>(color); }

    explicit operator std::string() const {
        return color == underlying::WHITE ? "w" : color == underlying::BLACK ? "b" : "NONE";
    }

    [[nodiscard]] constexpr underlying internal() const noexcept { return color; }

    friend std::ostream& operator<<(std::ostream& os, const Color& color) {
        return os << static_cast<std::string>(color);
    }

    static constexpr underlying WHITE = underlying::WHITE;
    static constexpr underlying BLACK = underlying::BLACK;
    static constexpr underlying NONE  = underlying::NONE;

   private:
    underlying color;

    static constexpr bool isValid(int c) { return c == 0 || c == 1 || c == -1; }
};

constexpr Color::underlying operator~(Color::underlying color) {
    return color == Color::underlying::WHITE   ? Color::underlying::BLACK
           : color == Color::underlying::BLACK ? Color::underlying::WHITE
                                               : Color::underlying::NONE;
}

}  // namespace chess

#include <vector>

namespace chess {
namespace utils {

// Split a string by a delimiter
[[nodiscard]] inline std::vector<std::string_view> splitString(std::string_view string, const char &delimiter) {
    std::vector<std::string_view> result;
    size_t start = 0;
    size_t end   = string.find(delimiter);

    while (end != std::string_view::npos) {
        result.push_back(string.substr(start, end - start));
        start = end + 1;
        end   = string.find(delimiter, start);
    }

    // Add the last chunk (or the only chunk if there are no delimiters)
    result.push_back(string.substr(start));

    return result;
}

constexpr char tolower(char c) { return (c >= 'A' && c <= 'Z') ? (c - 'A' + 'a') : c; }

}  // namespace utils

}  // namespace chess

namespace chess {

#define CHESS_DECLARE_RANK(N)                            \
    static constexpr auto SQ_A##N = underlying::SQ_A##N; \
    static constexpr auto SQ_B##N = underlying::SQ_B##N; \
    static constexpr auto SQ_C##N = underlying::SQ_C##N; \
    static constexpr auto SQ_D##N = underlying::SQ_D##N; \
    static constexpr auto SQ_E##N = underlying::SQ_E##N; \
    static constexpr auto SQ_F##N = underlying::SQ_F##N; \
    static constexpr auto SQ_G##N = underlying::SQ_G##N; \
    static constexpr auto SQ_H##N = underlying::SQ_H##N;

class File {
   public:
    enum class underlying : std::uint8_t { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, NO_FILE };

    constexpr File() : file(underlying::NO_FILE) {}
    constexpr File(underlying file) : file(file) {}
    constexpr File(int file) : file(static_cast<underlying>(file)) {}
    constexpr File(std::string_view sw)
        : file(static_cast<underlying>(static_cast<char>(utils::tolower(static_cast<unsigned char>(sw[0]))) - 'a')) {}

    [[nodiscard]] constexpr underlying internal() const noexcept { return file; }

    constexpr bool operator==(const File& rhs) const noexcept { return file == rhs.file; }
    constexpr bool operator!=(const File& rhs) const noexcept { return file != rhs.file; }

    constexpr bool operator==(const underlying& rhs) const noexcept { return file == rhs; }
    constexpr bool operator!=(const underlying& rhs) const noexcept { return file != rhs; }

    constexpr bool operator>=(const File& rhs) const noexcept {
        return static_cast<int>(file) >= static_cast<int>(rhs.file);
    }
    constexpr bool operator<=(const File& rhs) const noexcept {
        return static_cast<int>(file) <= static_cast<int>(rhs.file);
    }

    constexpr bool operator>(const File& rhs) const noexcept {
        return static_cast<int>(file) > static_cast<int>(rhs.file);
    }

    constexpr bool operator<(const File& rhs) const noexcept {
        return static_cast<int>(file) < static_cast<int>(rhs.file);
    }

    constexpr File& operator+=(int rhs) noexcept {
        file = underlying(static_cast<int>(file) + rhs);
        return *this;
    }

    constexpr operator int() const noexcept { return static_cast<int>(file); }

    explicit operator std::string() const { return std::string(1, static_cast<char>(static_cast<int>(file) + 'a')); }

    static constexpr underlying FILE_A  = underlying::FILE_A;
    static constexpr underlying FILE_B  = underlying::FILE_B;
    static constexpr underlying FILE_C  = underlying::FILE_C;
    static constexpr underlying FILE_D  = underlying::FILE_D;
    static constexpr underlying FILE_E  = underlying::FILE_E;
    static constexpr underlying FILE_F  = underlying::FILE_F;
    static constexpr underlying FILE_G  = underlying::FILE_G;
    static constexpr underlying FILE_H  = underlying::FILE_H;
    static constexpr underlying NO_FILE = underlying::NO_FILE;

   private:
    underlying file;
};

class Rank {
   public:
    enum class underlying { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, NO_RANK };

    constexpr Rank() : rank_(underlying::NO_RANK) {}
    constexpr Rank(underlying rank) : rank_(rank) {}
    constexpr Rank(int rank) : rank_(static_cast<underlying>(rank)) {}
    constexpr Rank(std::string_view sw)
        : rank_(static_cast<underlying>(static_cast<char>(utils::tolower(static_cast<unsigned char>(sw[0]))) - '1')) {}

    [[nodiscard]] constexpr underlying internal() const noexcept { return rank_; }

    constexpr bool operator==(const Rank& rhs) const noexcept { return rank_ == rhs.rank_; }
    constexpr bool operator!=(const Rank& rhs) const noexcept { return rank_ != rhs.rank_; }

    constexpr bool operator==(const underlying& rhs) const noexcept { return rank_ == rhs; }
    constexpr bool operator!=(const underlying& rhs) const noexcept { return rank_ != rhs; }

    constexpr bool operator>=(const Rank& rhs) const noexcept {
        return static_cast<int>(rank_) >= static_cast<int>(rhs.rank_);
    }
    constexpr bool operator<=(const Rank& rhs) const noexcept {
        return static_cast<int>(rank_) <= static_cast<int>(rhs.rank_);
    }

    constexpr Rank& operator+=(int rhs) noexcept {
        rank_ = underlying(static_cast<int>(rank_) + rhs);
        return *this;
    }

    operator std::string() const { return std::string(1, static_cast<char>(static_cast<int>(rank_) + '1')); }

    constexpr operator int() const noexcept { return static_cast<int>(rank_); }

    [[nodiscard]] constexpr std::uint64_t bb() const noexcept { return 0xffULL << (8 * static_cast<int>(rank_)); }

    [[nodiscard]] static constexpr bool back_rank(Rank r, Color color) noexcept {
        return r == Rank(static_cast<int>(color) * 7);
    }

    [[nodiscard]] static constexpr Rank rank(Rank r, Color color) noexcept {
        return Rank((static_cast<int>(r) ^ (static_cast<int>(color) * 7)));
    }

    static constexpr underlying RANK_1  = underlying::RANK_1;
    static constexpr underlying RANK_2  = underlying::RANK_2;
    static constexpr underlying RANK_3  = underlying::RANK_3;
    static constexpr underlying RANK_4  = underlying::RANK_4;
    static constexpr underlying RANK_5  = underlying::RANK_5;
    static constexpr underlying RANK_6  = underlying::RANK_6;
    static constexpr underlying RANK_7  = underlying::RANK_7;
    static constexpr underlying RANK_8  = underlying::RANK_8;
    static constexpr underlying NO_RANK = underlying::NO_RANK;

   private:
    underlying rank_;
};

class Square {
   public:
    // clang-format off
    enum class underlying {
        SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
        SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
        SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
        SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
        SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
        SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
        SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
        SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,
        NO_SQ
    };
    // clang-format on

// when c++20
#if __cplusplus >= 202002L || (defined(_MSC_VER) && _MSVC_LANG >= 202002L)
    using enum underlying;
#else

    CHESS_DECLARE_RANK(1)
    CHESS_DECLARE_RANK(2)
    CHESS_DECLARE_RANK(3)
    CHESS_DECLARE_RANK(4)
    CHESS_DECLARE_RANK(5)
    CHESS_DECLARE_RANK(6)
    CHESS_DECLARE_RANK(7)
    CHESS_DECLARE_RANK(8)

    static constexpr auto NO_SQ = underlying::NO_SQ;

#endif

    constexpr Square() : sq(underlying::NO_SQ) {}

    constexpr Square(int sq) : sq(static_cast<underlying>(sq)) { assert(sq <= 64 && sq >= 0); }
    constexpr Square(File file, Rank rank) : sq(static_cast<underlying>(file + rank * 8)) {}
    constexpr Square(Rank rank, File file) : sq(static_cast<underlying>(file + rank * 8)) {}
    constexpr Square(underlying sq) : sq(sq) {}
    constexpr Square(std::string_view str) : sq(static_cast<underlying>((str[0] - 'a') + (str[1] - '1') * 8)) {
        assert(str.size() >= 2);
    }

    constexpr Square operator^(const Square& s) const noexcept {
        return Square(static_cast<underlying>(static_cast<int>(sq) ^ s.index()));
    };

    constexpr bool operator==(const Square& rhs) const noexcept { return sq == rhs.sq; }

    constexpr bool operator!=(const Square& rhs) const noexcept { return sq != rhs.sq; }

    constexpr bool operator>(const Square& rhs) const noexcept {
        return static_cast<int>(sq) > static_cast<int>(rhs.sq);
    }

    constexpr bool operator>=(const Square& rhs) const noexcept {
        return static_cast<int>(sq) >= static_cast<int>(rhs.sq);
    }

    constexpr bool operator<(const Square& rhs) const noexcept {
        return static_cast<int>(sq) < static_cast<int>(rhs.sq);
    }

    constexpr bool operator<=(const Square& rhs) const noexcept {
        return static_cast<int>(sq) <= static_cast<int>(rhs.sq);
    }

    constexpr Square operator+(const Square& rhs) const noexcept {
        return Square(static_cast<underlying>(static_cast<int>(sq) + static_cast<int>(rhs.sq)));
    }

    constexpr Square operator-(const Square& rhs) const noexcept {
        return Square(static_cast<underlying>(static_cast<int>(sq) - static_cast<int>(rhs.sq)));
    }

    constexpr Square& operator++() noexcept {
        sq = static_cast<underlying>(static_cast<int>(sq) + 1);
        return *this;
    }

    constexpr Square operator++(int) noexcept {
        Square tmp(*this);
        operator++();
        return tmp;
    }

    constexpr Square& operator--() noexcept {
        sq = static_cast<underlying>(static_cast<int>(sq) - 1);
        return *this;
    }

    constexpr Square operator--(int) noexcept {
        Square tmp(*this);
        operator--();
        return tmp;
    }

    /**
     * @brief Get a string representation of the square.
     */
    [[nodiscard]] operator std::string() const {
        std::string str;
        str += static_cast<std::string>(file());
        str += static_cast<std::string>(rank());
        return str;
    }

    [[nodiscard]] constexpr int index() const noexcept { return static_cast<int>(sq); }

    [[nodiscard]] constexpr File file() const noexcept { return File(index() & 7); }
    [[nodiscard]] constexpr Rank rank() const noexcept { return Rank(index() >> 3); }

    /**
     * @brief Check if the square is light.
     * @return
     */
    [[nodiscard]] constexpr bool is_light() const noexcept { return (file() + rank()) & 1;
    }

    /**
     * @brief Check if the square is dark.
     * @return
     */
    [[nodiscard]] constexpr bool is_dark() const noexcept { return !is_light(); }

    /**
     * @brief Check if the square is vali.d
     * @return
     */
    [[nodiscard]] constexpr bool is_valid() const noexcept { return static_cast<std::int8_t>(sq) < 64; }

    /**
     * @brief Check if the square is valid.
     * @param r
     * @param f
     * @return
     */
    [[nodiscard]] constexpr static bool is_valid(Rank r, File f) noexcept {
        return r >= Rank::RANK_1 && r <= Rank::RANK_8 && f >= File::FILE_A && f <= File::FILE_H;
    }

    /**
     * @brief Get the chebyshev distance between two squares.
     * @param sq
     * @param sq2
     * @return
     */
    [[nodiscard]] static int distance(Square sq, Square sq2) noexcept {
        return std::max(std::abs(sq.file() - sq2.file()), std::abs(sq.rank() - sq2.rank()));
    }

    /**
     * @brief Absolute value of sq - sq2.
     * @param sq
     * @param sq2
     * @return
     */
    [[nodiscard]] static int value_distance(Square sq, Square sq2) noexcept {
        return std::abs(sq.index() - sq2.index());
    }

    /**
     * @brief Check if the squares share the same color. I.e. if they are both light or dark.
     * @param sq
     * @param sq2
     * @return
     */
    [[nodiscard]] static constexpr bool same_color(Square sq, Square sq2) noexcept {
        return ((9 * (sq ^ sq2).index()) & 8) == 0;
    }

    /**
     * @brief Check if the square is on the back rank.
     * @param sq
     * @param color
     * @return
     */
    [[nodiscard]] static constexpr bool back_rank(Square sq, Color color) noexcept {
        return Rank::back_rank(sq.rank(), color);
    }

    /**
     * @brief Flips the square vertically.
     * @return
     */
    constexpr Square& flip() noexcept {
        sq = static_cast<underlying>(static_cast<int>(sq) ^ 56);
        return *this;
    }

    /**
     * @brief Flips the square vertically, depending on the color.
     * @param c
     * @return
     */
    [[nodiscard]] constexpr Square relative_square(Color c) const noexcept {
        return Square(static_cast<int>(sq) ^ (static_cast<int>(c) * 56));
    }

    [[nodiscard]] constexpr int diagonal_of() const noexcept { return 7 + rank() - file(); }

    [[nodiscard]] constexpr int antidiagonal_of() const noexcept { return rank() + file(); }

    /**
     * @brief Get the en passant square. Should only be called for valid ep positions.
     * @return
     */
    [[nodiscard]] constexpr Square ep_square() const noexcept {
        assert(rank() == Rank::RANK_3     // capture
               || rank() == Rank::RANK_4  // push
               || rank() == Rank::RANK_5  // push
               || rank() == Rank::RANK_6  // capture
        );
        return Square(static_cast<int>(sq) ^ 8);
    }

    /**
     * @brief Get the destination square of the king after castling.
     * @param is_king_side
     * @param c
     * @return
     */
    [[nodiscard]] static constexpr Square castling_king_square(bool is_king_side, Color c) noexcept {
        return Square(is_king_side ? Square::SQ_G1 : Square::SQ_C1).relative_square(c);
    }

    /**
     * @brief Get the destination square of the rook after castling.
     * @param is_king_side
     * @param c
     * @return
     */
    [[nodiscard]] static constexpr Square castling_rook_square(bool is_king_side, Color c) noexcept {
        return Square(is_king_side ? Square::SQ_F1 : Square::SQ_D1).relative_square(c);
    }

    /**
     * @brief Maximum number of squares.
     * @return
     */
    [[nodiscard]] static constexpr int max() noexcept { return 64; }

    [[nodiscard]] static bool is_valid_string_sq(std::string_view str) noexcept {
        return str.size() == 2 && str[0] >= 'a' && str[0] <= 'h' && str[1] >= '1' && str[1] <= '8';
    }

    [[nodiscard]] static constexpr bool is_valid_sq(int sq) noexcept { return sq >= 0 && sq < 64; }

   private:
    underlying sq;
};

inline std::ostream& operator<<(std::ostream& os, const Square& sq) {
    os << static_cast<std::string>(sq);
    return os;
}

enum class Direction : std::int8_t {
    NORTH      = 8,
    WEST       = -1,
    SOUTH      = -8,
    EAST       = 1,
    NORTH_EAST = 9,
    NORTH_WEST = 7,
    SOUTH_WEST = -9,
    SOUTH_EAST = -7
};

[[nodiscard]] constexpr Direction make_direction(Direction dir, Color c) noexcept {
    if (c == Color::BLACK) return static_cast<Direction>(-static_cast<std::int8_t>(dir));
    return dir;
}

constexpr Square operator+(Square sq, Direction dir) {
    return static_cast<Square>(sq.index() + static_cast<std::int8_t>(dir));
}

#undef CHESS_DECLARE_RANK

}  // namespace chess

namespace chess {

class Bitboard {
   public:
    constexpr Bitboard() : bits(0) {}
    constexpr Bitboard(std::uint64_t bits) : bits(bits) {}
    constexpr Bitboard(File file) : bits(0) {
        assert(file != File::NO_FILE);
        bits = 0x0101010101010101ULL << static_cast<int>(file.internal());
    }
    constexpr Bitboard(Rank rank) : bits(0) {
        assert(rank != Rank::NO_RANK);
        bits = 0xFFULL << (8 * static_cast<int>(rank.internal()));
    }

    explicit operator bool() const noexcept { return bits != 0; }

    explicit operator std::string() const {
        std::bitset<64> b(bits);
        std::string str_bitset = b.to_string();

        std::string str;

        for (int i = 0; i < 64; i += 8) {
            std::string x = str_bitset.substr(i, 8);
            std::reverse(x.begin(), x.end());
            str += x + '\n';
        }
        return str;
    }

    constexpr Bitboard operator&&(bool rhs) const noexcept { return Bitboard(bits && rhs); }

    constexpr Bitboard operator&(std::uint64_t rhs) const noexcept { return Bitboard(bits & rhs); }
    constexpr Bitboard operator|(std::uint64_t rhs) const noexcept { return Bitboard(bits | rhs); }
    constexpr Bitboard operator^(std::uint64_t rhs) const noexcept { return Bitboard(bits ^ rhs); }
    constexpr Bitboard operator<<(std::uint64_t rhs) const noexcept { return Bitboard(bits << rhs); }
    constexpr Bitboard operator>>(std::uint64_t rhs) const noexcept { return Bitboard(bits >> rhs); }
    constexpr bool operator==(std::uint64_t rhs) const noexcept { return bits == rhs; }
    constexpr bool operator!=(std::uint64_t rhs) const noexcept { return bits != rhs; }

    constexpr Bitboard operator&(const Bitboard& rhs) const noexcept { return Bitboard(bits & rhs.bits); }
    constexpr Bitboard operator|(const Bitboard& rhs) const noexcept { return Bitboard(bits | rhs.bits); }
    constexpr Bitboard operator^(const Bitboard& rhs) const noexcept { return Bitboard(bits ^ rhs.bits); }
    constexpr Bitboard operator~() const noexcept { return Bitboard(~bits); }

    constexpr Bitboard& operator&=(const Bitboard& rhs) noexcept {
        bits &= rhs.bits;
        return *this;
    }

    constexpr Bitboard& operator|=(const Bitboard& rhs) noexcept {
        bits |= rhs.bits;
        return *this;
    }

    constexpr Bitboard& operator^=(const Bitboard& rhs) noexcept {
        bits ^= rhs.bits;
        return *this;
    }

    constexpr bool operator==(const Bitboard& rhs) const noexcept { return bits == rhs.bits; }
    constexpr bool operator!=(const Bitboard& rhs) const noexcept { return bits != rhs.bits; }
    constexpr bool operator||(const Bitboard& rhs) const noexcept { return bits || rhs.bits; }
    constexpr bool operator&&(const Bitboard& rhs) const noexcept { return bits && rhs.bits; }

    constexpr Bitboard& set(int index) noexcept {
        assert(index >= 0 && index < 64);
        bits |= (1ULL << index);
        return *this;
    }

    [[nodiscard]] constexpr bool check(int index) const noexcept {
        assert(index >= 0 && index < 64);
        return bits & (1ULL << index);
    }

    constexpr Bitboard& clear(int index) noexcept {
        assert(index >= 0 && index < 64);
        bits &= ~(1ULL << index);
        return *this;
    }

    constexpr Bitboard& clear() noexcept {
        bits = 0;
        return *this;
    }

    [[nodiscard]] static constexpr Bitboard fromSquare(int index) noexcept {
        assert(index >= 0 && index < 64);
        return Bitboard(1ULL << index);
    }

    [[nodiscard]] static constexpr Bitboard fromSquare(Square sq) noexcept {
        assert(sq.index() >= 0 && sq.index() < 64);
        return Bitboard(1ULL << sq.index());
    }

    [[nodiscard]] constexpr bool empty() const noexcept { return bits == 0; }

    [[nodiscard]]
#if __cpp_lib_bitops >= 201907L
    constexpr
#endif
        int lsb() const noexcept {
        assert(bits != 0);
#if __cpp_lib_bitops >= 201907L
        return std::countr_zero(bits);
#else
#    if defined(__GNUC__)
        return __builtin_ctzll(bits);
#    elif defined(_MSC_VER)
        unsigned long idx;
        _BitScanForward64(&idx, bits);
        return static_cast<int>(idx);
#    else
#        error "Compiler not supported."
#    endif
#endif
    }

    [[nodiscard]]
#if __cpp_lib_bitops >= 201907L
    constexpr
#endif
        int msb() const noexcept {
        assert(bits != 0);

#if __cpp_lib_bitops >= 201907L
        return std::countl_zero(bits) ^ 63;
#else
#    if defined(__GNUC__)
        return 63 ^ __builtin_clzll(bits);
#    elif defined(_MSC_VER)
        unsigned long idx;
        _BitScanReverse64(&idx, bits);
        return static_cast<int>(idx);
#    else
#        error "Compiler not supported."
#    endif
#endif
    }

    [[nodiscard]]
#if __cpp_lib_bitops >= 201907L
    constexpr
#endif
        int count() const noexcept {
#if __cpp_lib_bitops >= 201907L
        return std::popcount(bits);
#else
#    if defined(_MSC_VER) || defined(__INTEL_COMPILER)
        return static_cast<int>(_mm_popcnt_u64(bits));
#    else
        return __builtin_popcountll(bits);
#    endif
#endif
    }

    [[nodiscard]]
#if __cpp_lib_bitops >= 201907L
    constexpr
#endif
        std::uint8_t pop() noexcept {
        assert(bits != 0);
        std::uint8_t index = lsb();
        bits &= bits - 1;
        return index;
    }

    [[nodiscard]] constexpr std::uint64_t getBits() const noexcept { return bits; }

    friend std::ostream& operator<<(std::ostream& os, const Bitboard& bb);

   private:
    std::uint64_t bits;
};

inline std::ostream& operator<<(std::ostream& os, const Bitboard& bb) {
    os << std::string(bb);
    return os;
}

constexpr Bitboard operator&(std::uint64_t lhs, const Bitboard& rhs) { return rhs & lhs; }
constexpr Bitboard operator|(std::uint64_t lhs, const Bitboard& rhs) { return rhs | lhs; }
}  // namespace chess

namespace chess {
class Board;
}  // namespace chess



namespace chess {

class PieceType {
   public:
    enum class underlying : std::uint8_t {
        PAWN,
        KNIGHT,
        BISHOP,
        ROOK,
        QUEEN,
        KING,
        NONE,
    };

    constexpr PieceType() : pt(underlying::NONE) {}
    constexpr PieceType(underlying pt) : pt(pt) {}
    constexpr explicit PieceType(std::string_view type) : pt(underlying::NONE) {
        assert(type.size() > 0);

        char c = type[0];

        if (c == 'P' || c == 'p')
            pt = underlying::PAWN;
        else if (c == 'N' || c == 'n')
            pt = underlying::KNIGHT;
        else if (c == 'B' || c == 'b')
            pt = underlying::BISHOP;
        else if (c == 'R' || c == 'r')
            pt = underlying::ROOK;
        else if (c == 'Q' || c == 'q')
            pt = underlying::QUEEN;
        else if (c == 'K' || c == 'k')
            pt = underlying::KING;
        else
            pt = underlying::NONE;
    }

    explicit operator std::string() const {
        if (pt == underlying::NONE) return " ";
        constexpr static const char* pieceTypeStr[] = {"p", "n", "b", "r", "q", "k"};
        return pieceTypeStr[static_cast<int>(pt)];
    }

    constexpr bool operator==(const PieceType& rhs) const noexcept { return pt == rhs.pt; }
    constexpr bool operator!=(const PieceType& rhs) const noexcept { return pt != rhs.pt; }

    constexpr operator int() const noexcept { return static_cast<int>(pt); }

    [[nodiscard]] constexpr underlying internal() const noexcept { return pt; }

    static constexpr underlying PAWN   = underlying::PAWN;
    static constexpr underlying KNIGHT = underlying::KNIGHT;
    static constexpr underlying BISHOP = underlying::BISHOP;
    static constexpr underlying ROOK   = underlying::ROOK;
    static constexpr underlying QUEEN  = underlying::QUEEN;
    static constexpr underlying KING   = underlying::KING;
    static constexpr underlying NONE   = underlying::NONE;

   private:
    underlying pt;
};

inline std::ostream& operator<<(std::ostream& os, const PieceType& pt) {
    os << static_cast<std::string>(pt);
    return os;
}

class Piece {
   public:
    enum class underlying : std::uint8_t {
        WHITEPAWN,
        WHITEKNIGHT,
        WHITEBISHOP,
        WHITEROOK,
        WHITEQUEEN,
        WHITEKING,
        BLACKPAWN,
        BLACKKNIGHT,
        BLACKBISHOP,
        BLACKROOK,
        BLACKQUEEN,
        BLACKKING,
        NONE
    };

    constexpr Piece() : piece(underlying::NONE) {}
    constexpr Piece(underlying piece) : piece(piece) {}
    constexpr Piece(PieceType type, Color color)
        : piece(color == Color::NONE      ? Piece::NONE
                : type == PieceType::NONE ? Piece::NONE
                                          : static_cast<underlying>(static_cast<int>(color.internal()) * 6 + type)) {}
    constexpr Piece(Color color, PieceType type)
        : piece(color == Color::NONE      ? Piece::NONE
                : type == PieceType::NONE ? Piece::NONE
                                          : static_cast<underlying>(static_cast<int>(color.internal()) * 6 + type)) {}
    constexpr Piece(std::string_view p) : piece(underlying::NONE) { piece = convertCharToUnderlying(p[0]); }

    constexpr bool operator<(const Piece& rhs) const noexcept { return piece < rhs.piece; }
    constexpr bool operator>(const Piece& rhs) const noexcept { return piece > rhs.piece; }
    constexpr bool operator==(const Piece& rhs) const noexcept { return piece == rhs.piece; }
    constexpr bool operator!=(const Piece& rhs) const noexcept { return piece != rhs.piece; }

    constexpr bool operator==(const underlying& rhs) const noexcept { return piece == rhs; }
    constexpr bool operator!=(const underlying& rhs) const noexcept { return piece != rhs; }

    constexpr bool operator==(const PieceType& rhs) const noexcept { return type() == rhs; }
    constexpr bool operator!=(const PieceType& rhs) const noexcept { return type() != rhs; }

    explicit operator std::string() const {
        constexpr static const char* pieceStr[] = {"P", "N", "B", "R", "Q", "K",  //
                                                   "p", "n", "b", "r", "q", "k"};
        if (piece == NONE) return ".";
        return pieceStr[static_cast<int>(piece)];
    }

    constexpr operator int() const noexcept { return static_cast<int>(piece); }

    [[nodiscard]] constexpr PieceType type() const noexcept {
        if (piece == NONE) return PieceType::NONE;
        // return static_cast<PieceType::underlying>(int(piece) % 6);
        return static_cast<PieceType::underlying>(static_cast<int>(piece) > 5 ? static_cast<int>(piece) - 6
                                                                              : static_cast<int>(piece));
    }

    [[nodiscard]] constexpr Color color() const noexcept {
        if (piece == NONE) return Color::NONE;
        return static_cast<Color>(static_cast<int>(piece) / 6);
    }

    [[nodiscard]] constexpr underlying internal() const noexcept { return piece; }

    static constexpr underlying NONE        = underlying::NONE;
    static constexpr underlying WHITEPAWN   = underlying::WHITEPAWN;
    static constexpr underlying WHITEKNIGHT = underlying::WHITEKNIGHT;
    static constexpr underlying WHITEBISHOP = underlying::WHITEBISHOP;
    static constexpr underlying WHITEROOK   = underlying::WHITEROOK;
    static constexpr underlying WHITEQUEEN  = underlying::WHITEQUEEN;
    static constexpr underlying WHITEKING   = underlying::WHITEKING;
    static constexpr underlying BLACKPAWN   = underlying::BLACKPAWN;
    static constexpr underlying BLACKKNIGHT = underlying::BLACKKNIGHT;
    static constexpr underlying BLACKBISHOP = underlying::BLACKBISHOP;
    static constexpr underlying BLACKROOK   = underlying::BLACKROOK;
    static constexpr underlying BLACKQUEEN  = underlying::BLACKQUEEN;
    static constexpr underlying BLACKKING   = underlying::BLACKKING;

   private:
    underlying piece;

    [[nodiscard]] constexpr static underlying convertCharToUnderlying(char c) {
        switch (c) {
            case 'P':
                return WHITEPAWN;
            case 'N':
                return WHITEKNIGHT;
            case 'B':
                return WHITEBISHOP;
            case 'R':
                return WHITEROOK;
            case 'Q':
                return WHITEQUEEN;
            case 'K':
                return WHITEKING;
            case 'p':
                return BLACKPAWN;
            case 'n':
                return BLACKKNIGHT;
            case 'b':
                return BLACKBISHOP;
            case 'r':
                return BLACKROOK;
            case 'q':
                return BLACKQUEEN;
            case 'k':
                return BLACKKING;
            default:
                return NONE;
        }
    }
};
}  // namespace chess

namespace chess {
class attacks {
    using U64 = std::uint64_t;

#ifdef CHESS_USE_PEXT
    struct Magic {
        U64 mask;
        Bitboard *attacks;
        U64 operator()(Bitboard b) const noexcept { return _pext_u64(b.getBits(), mask); }
    };
#else
    struct Magic {
        U64 mask;
        U64 magic;
        Bitboard *attacks;
        U64 shift;
        U64 operator()(Bitboard b) const noexcept { return (((b & mask)).getBits() * magic) >> shift; }
    };
#endif

    // Slow function to calculate bishop and rook attacks
    template <bool ISROOK>
    [[nodiscard]] static Bitboard sliderAttacks(Square sq, Bitboard occupied) noexcept;

    // Initializes the magic bitboard tables for sliding pieces
    static void initSliders(Square sq, Magic table[], U64 magic,
                            const std::function<Bitboard(Square, Bitboard)> &attacks);

    // clang-format off
    // pre-calculated lookup table for pawn attacks
    static constexpr Bitboard PawnAttacks[2][64] = {
        // white pawn attacks
        { 0x200, 0x500, 0xa00, 0x1400,
        0x2800, 0x5000, 0xa000, 0x4000,
        0x20000, 0x50000, 0xa0000, 0x140000,
        0x280000, 0x500000, 0xa00000, 0x400000,
        0x2000000, 0x5000000, 0xa000000, 0x14000000,
        0x28000000, 0x50000000, 0xa0000000, 0x40000000,
        0x200000000, 0x500000000, 0xa00000000, 0x1400000000,
        0x2800000000, 0x5000000000, 0xa000000000, 0x4000000000,
        0x20000000000, 0x50000000000, 0xa0000000000, 0x140000000000,
        0x280000000000, 0x500000000000, 0xa00000000000, 0x400000000000,
        0x2000000000000, 0x5000000000000, 0xa000000000000, 0x14000000000000,
        0x28000000000000, 0x50000000000000, 0xa0000000000000, 0x40000000000000,
        0x200000000000000, 0x500000000000000, 0xa00000000000000, 0x1400000000000000,
        0x2800000000000000, 0x5000000000000000, 0xa000000000000000, 0x4000000000000000,
        0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0 },

        // black pawn attacks
        { 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0,
            0x2, 0x5, 0xa, 0x14,
            0x28, 0x50, 0xa0, 0x40,
            0x200, 0x500, 0xa00, 0x1400,
            0x2800, 0x5000, 0xa000, 0x4000,
            0x20000, 0x50000, 0xa0000, 0x140000,
            0x280000, 0x500000, 0xa00000, 0x400000,
            0x2000000, 0x5000000, 0xa000000, 0x14000000,
            0x28000000, 0x50000000, 0xa0000000, 0x40000000,
            0x200000000, 0x500000000, 0xa00000000, 0x1400000000,
            0x2800000000, 0x5000000000, 0xa000000000, 0x4000000000,
            0x20000000000, 0x50000000000, 0xa0000000000, 0x140000000000,
            0x280000000000, 0x500000000000, 0xa00000000000, 0x400000000000,
            0x2000000000000, 0x5000000000000, 0xa000000000000, 0x14000000000000,
            0x28000000000000, 0x50000000000000, 0xa0000000000000, 0x40000000000000
        }
    };

    // clang-format on

    // pre-calculated lookup table for knight attacks
    static constexpr Bitboard KnightAttacks[64] = {
        0x0000000000020400, 0x0000000000050800, 0x00000000000A1100, 0x0000000000142200, 0x0000000000284400,
        0x0000000000508800, 0x0000000000A01000, 0x0000000000402000, 0x0000000002040004, 0x0000000005080008,
        0x000000000A110011, 0x0000000014220022, 0x0000000028440044, 0x0000000050880088, 0x00000000A0100010,
        0x0000000040200020, 0x0000000204000402, 0x0000000508000805, 0x0000000A1100110A, 0x0000001422002214,
        0x0000002844004428, 0x0000005088008850, 0x000000A0100010A0, 0x0000004020002040, 0x0000020400040200,
        0x0000050800080500, 0x00000A1100110A00, 0x0000142200221400, 0x0000284400442800, 0x0000508800885000,
        0x0000A0100010A000, 0x0000402000204000, 0x0002040004020000, 0x0005080008050000, 0x000A1100110A0000,
        0x0014220022140000, 0x0028440044280000, 0x0050880088500000, 0x00A0100010A00000, 0x0040200020400000,
        0x0204000402000000, 0x0508000805000000, 0x0A1100110A000000, 0x1422002214000000, 0x2844004428000000,
        0x5088008850000000, 0xA0100010A0000000, 0x4020002040000000, 0x0400040200000000, 0x0800080500000000,
        0x1100110A00000000, 0x2200221400000000, 0x4400442800000000, 0x8800885000000000, 0x100010A000000000,
        0x2000204000000000, 0x0004020000000000, 0x0008050000000000, 0x00110A0000000000, 0x0022140000000000,
        0x0044280000000000, 0x0088500000000000, 0x0010A00000000000, 0x0020400000000000};

    // pre-calculated lookup table for king attacks
    static constexpr Bitboard KingAttacks[64] = {
        0x0000000000000302, 0x0000000000000705, 0x0000000000000E0A, 0x0000000000001C14, 0x0000000000003828,
        0x0000000000007050, 0x000000000000E0A0, 0x000000000000C040, 0x0000000000030203, 0x0000000000070507,
        0x00000000000E0A0E, 0x00000000001C141C, 0x0000000000382838, 0x0000000000705070, 0x0000000000E0A0E0,
        0x0000000000C040C0, 0x0000000003020300, 0x0000000007050700, 0x000000000E0A0E00, 0x000000001C141C00,
        0x0000000038283800, 0x0000000070507000, 0x00000000E0A0E000, 0x00000000C040C000, 0x0000000302030000,
        0x0000000705070000, 0x0000000E0A0E0000, 0x0000001C141C0000, 0x0000003828380000, 0x0000007050700000,
        0x000000E0A0E00000, 0x000000C040C00000, 0x0000030203000000, 0x0000070507000000, 0x00000E0A0E000000,
        0x00001C141C000000, 0x0000382838000000, 0x0000705070000000, 0x0000E0A0E0000000, 0x0000C040C0000000,
        0x0003020300000000, 0x0007050700000000, 0x000E0A0E00000000, 0x001C141C00000000, 0x0038283800000000,
        0x0070507000000000, 0x00E0A0E000000000, 0x00C040C000000000, 0x0302030000000000, 0x0705070000000000,
        0x0E0A0E0000000000, 0x1C141C0000000000, 0x3828380000000000, 0x7050700000000000, 0xE0A0E00000000000,
        0xC040C00000000000, 0x0203000000000000, 0x0507000000000000, 0x0A0E000000000000, 0x141C000000000000,
        0x2838000000000000, 0x5070000000000000, 0xA0E0000000000000, 0x40C0000000000000};

    static constexpr U64 RookMagics[64] = {
        0x8a80104000800020ULL, 0x140002000100040ULL,  0x2801880a0017001ULL,  0x100081001000420ULL,
        0x200020010080420ULL,  0x3001c0002010008ULL,  0x8480008002000100ULL, 0x2080088004402900ULL,
        0x800098204000ULL,     0x2024401000200040ULL, 0x100802000801000ULL,  0x120800800801000ULL,
        0x208808088000400ULL,  0x2802200800400ULL,    0x2200800100020080ULL, 0x801000060821100ULL,
        0x80044006422000ULL,   0x100808020004000ULL,  0x12108a0010204200ULL, 0x140848010000802ULL,
        0x481828014002800ULL,  0x8094004002004100ULL, 0x4010040010010802ULL, 0x20008806104ULL,
        0x100400080208000ULL,  0x2040002120081000ULL, 0x21200680100081ULL,   0x20100080080080ULL,
        0x2000a00200410ULL,    0x20080800400ULL,      0x80088400100102ULL,   0x80004600042881ULL,
        0x4040008040800020ULL, 0x440003000200801ULL,  0x4200011004500ULL,    0x188020010100100ULL,
        0x14800401802800ULL,   0x2080040080800200ULL, 0x124080204001001ULL,  0x200046502000484ULL,
        0x480400080088020ULL,  0x1000422010034000ULL, 0x30200100110040ULL,   0x100021010009ULL,
        0x2002080100110004ULL, 0x202008004008002ULL,  0x20020004010100ULL,   0x2048440040820001ULL,
        0x101002200408200ULL,  0x40802000401080ULL,   0x4008142004410100ULL, 0x2060820c0120200ULL,
        0x1001004080100ULL,    0x20c020080040080ULL,  0x2935610830022400ULL, 0x44440041009200ULL,
        0x280001040802101ULL,  0x2100190040002085ULL, 0x80c0084100102001ULL, 0x4024081001000421ULL,
        0x20030a0244872ULL,    0x12001008414402ULL,   0x2006104900a0804ULL,  0x1004081002402ULL};

    static constexpr U64 BishopMagics[64] = {
        0x40040844404084ULL,   0x2004208a004208ULL,   0x10190041080202ULL,   0x108060845042010ULL,
        0x581104180800210ULL,  0x2112080446200010ULL, 0x1080820820060210ULL, 0x3c0808410220200ULL,
        0x4050404440404ULL,    0x21001420088ULL,      0x24d0080801082102ULL, 0x1020a0a020400ULL,
        0x40308200402ULL,      0x4011002100800ULL,    0x401484104104005ULL,  0x801010402020200ULL,
        0x400210c3880100ULL,   0x404022024108200ULL,  0x810018200204102ULL,  0x4002801a02003ULL,
        0x85040820080400ULL,   0x810102c808880400ULL, 0xe900410884800ULL,    0x8002020480840102ULL,
        0x220200865090201ULL,  0x2010100a02021202ULL, 0x152048408022401ULL,  0x20080002081110ULL,
        0x4001001021004000ULL, 0x800040400a011002ULL, 0xe4004081011002ULL,   0x1c004001012080ULL,
        0x8004200962a00220ULL, 0x8422100208500202ULL, 0x2000402200300c08ULL, 0x8646020080080080ULL,
        0x80020a0200100808ULL, 0x2010004880111000ULL, 0x623000a080011400ULL, 0x42008c0340209202ULL,
        0x209188240001000ULL,  0x400408a884001800ULL, 0x110400a6080400ULL,   0x1840060a44020800ULL,
        0x90080104000041ULL,   0x201011000808101ULL,  0x1a2208080504f080ULL, 0x8012020600211212ULL,
        0x500861011240000ULL,  0x180806108200800ULL,  0x4000020e01040044ULL, 0x300000261044000aULL,
        0x802241102020002ULL,  0x20906061210001ULL,   0x5a84841004010310ULL, 0x4010801011c04ULL,
        0xa010109502200ULL,    0x4a02012000ULL,       0x500201010098b028ULL, 0x8040002811040900ULL,
        0x28000010020204ULL,   0x6000020202d0240ULL,  0x8918844842082200ULL, 0x4010011029020020ULL};

    static inline Bitboard RookAttacks[0x19000]  = {};
    static inline Bitboard BishopAttacks[0x1480] = {};

    static inline Magic RookTable[64]   = {};
    static inline Magic BishopTable[64] = {};

   public:
    static constexpr Bitboard MASK_RANK[8] = {0xff,         0xff00,         0xff0000,         0xff000000,
                                              0xff00000000, 0xff0000000000, 0xff000000000000, 0xff00000000000000};

    static constexpr Bitboard MASK_FILE[8] = {
        0x101010101010101,  0x202020202020202,  0x404040404040404,  0x808080808080808,
        0x1010101010101010, 0x2020202020202020, 0x4040404040404040, 0x8080808080808080,
    };

    /**
     * @brief  Shifts a bitboard in a given direction
     * @tparam direction
     * @param b
     * @return
     */
    template <Direction direction>
    [[nodiscard]] static constexpr Bitboard shift(const Bitboard b);

    /**
     * @brief
     * @tparam c
     * @param pawns
     * @return
     */
    template <Color::underlying c>
    [[nodiscard]] static Bitboard pawnLeftAttacks(const Bitboard pawns);

    /**
     * @brief Generate the right side pawn attacks.
     * @tparam c
     * @param pawns
     * @return
     */
    template <Color::underlying c>
    [[nodiscard]] static Bitboard pawnRightAttacks(const Bitboard pawns);

    /**
     * @brief Returns the pawn attacks for a given color and square
     * @param c
     * @param sq
     * @return
     */
    [[nodiscard]] static Bitboard pawn(Color c, Square sq) noexcept;

    /**
     * @brief Returns the knight attacks for a given square
     * @param sq
     * @return
     */
    [[nodiscard]] static Bitboard knight(Square sq) noexcept;

    /**
     * @brief Returns the bishop attacks for a given square
     * @param sq
     * @param occupied
     * @return
     */
    [[nodiscard]] static Bitboard bishop(Square sq, Bitboard occupied) noexcept;

    /**
     * @brief Returns the rook attacks for a given square
     * @param sq
     * @param occupied
     * @return
     */
    [[nodiscard]] static Bitboard rook(Square sq, Bitboard occupied) noexcept;

    /**
     * @brief Returns the queen attacks for a given square
     * @param sq
     * @param occupied
     * @return
     */
    [[nodiscard]] static Bitboard queen(Square sq, Bitboard occupied) noexcept;

    /**
     * @brief Returns the king attacks for a given square
     * @param sq
     * @return
     */
    [[nodiscard]] static Bitboard king(Square sq) noexcept;

    /**
     * @brief Returns the origin squares of pieces of a given color attacking a target square
     * @param board
     * @param color Attacker Color
     * @param square Attacked Square
     * @return
     */
    [[nodiscard]] static Bitboard attackers(const Board &board, Color color, Square square) noexcept;

    /**
     * @brief Returns the slider attacks for a given square
     * @param sq
     * @param occupied
     * @tparam pt
     * @return
     */
    template <PieceType::underlying pt>
    [[nodiscard]] static Bitboard slider(Square sq, Bitboard occupied) noexcept;

    /**
     * @brief [Internal Usage] Initializes the attacks for the bishop and rook. Called once at startup.
     */
    static inline void initAttacks();
};
}  // namespace chess

#include <array>
#include <cctype>
#include <optional>

// check if charconv header is available
#if __has_include(<charconv>)
#    define CHESS_USE_CHARCONV
#    include <charconv>
#else
#    include <sstream>
#endif



namespace chess::constants {

constexpr Bitboard DEFAULT_CHECKMASK = Bitboard(0xFFFFFFFFFFFFFFFFull);
constexpr auto STARTPOS              = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
constexpr auto MAX_MOVES             = 256;
}  // namespace chess::constants



namespace chess {

class Move {
   public:
    Move() = default;
    constexpr Move(std::uint16_t move) : move_(move), score_(0) {}

    /**
     * @brief Creates a move from a source and target square.
     * @tparam MoveType
     * @param source
     * @param target
     * @param pt leave this empty if it is not a promotion move, otherwise pass the piece type of the new piece.
     * @return
     */
    template <std::uint16_t MoveType = 0>
    [[nodiscard]] static constexpr Move make(Square source, Square target, PieceType pt = PieceType::KNIGHT) noexcept {
        assert(pt >= PieceType(PieceType::KNIGHT) && pt <= PieceType(PieceType::QUEEN));

        std::uint16_t bits_promotion = static_cast<std::uint16_t>(pt - PieceType(PieceType::KNIGHT));

        return Move(MoveType + (bits_promotion << 12) + (source.index() << 6) + target.index());
    }

    /**
     * @brief Get the source square of the move.
     * @return
     */
    [[nodiscard]] constexpr Square from() const noexcept { return static_cast<Square>((move_ >> 6) & 0x3F); }

    /**
     * @brief Get the target square of the move.
     * @return
     */
    [[nodiscard]] constexpr Square to() const noexcept { return static_cast<Square>(move_ & 0x3F); }

    /**
     * @brief Get the type of the move. Can be NORMAL, PROMOTION, ENPASSANT or CASTLING.
     * @return
     */
    [[nodiscard]] constexpr std::uint16_t typeOf() const noexcept {
        return static_cast<std::uint16_t>(move_ & (3 << 14));
    }

    /**
     * @brief Get the promotion piece of the move, should only be used if typeOf() returns PROMOTION.
     * @return
     */
    [[nodiscard]] constexpr PieceType promotionType() const noexcept {
        return static_cast<PieceType::underlying>(((move_ >> 12) & 3) + PieceType(PieceType::KNIGHT));
    }

    /**
     * @brief Set the score for a move. Useful if you later want to sort the moves.
     * @param score
     */
    constexpr void setScore(std::int16_t score) noexcept { score_ = score; }

    [[nodiscard]] constexpr std::uint16_t move() const noexcept { return move_; }
    [[nodiscard]] constexpr std::int16_t score() const noexcept { return score_; }

    constexpr bool operator==(const Move &rhs) const noexcept { return move_ == rhs.move_; }
    constexpr bool operator!=(const Move &rhs) const noexcept { return move_ != rhs.move_; }

    static constexpr std::uint16_t NO_MOVE   = 0;
    static constexpr std::uint16_t NULL_MOVE = 65;
    static constexpr std::uint16_t NORMAL    = 0;
    static constexpr std::uint16_t PROMOTION = 1 << 14;
    static constexpr std::uint16_t ENPASSANT = 2 << 14;
    static constexpr std::uint16_t CASTLING  = 3 << 14;

   private:
    std::uint16_t move_;
    std::int16_t score_;
};

}  // namespace chess



#include <cstddef>
#include <iterator>
#include <stdexcept>


namespace chess {
class Movelist {
   public:
    using value_type      = Move;
    using size_type       = int;
    using difference_type = std::ptrdiff_t;
    using reference       = value_type&;
    using const_reference = const value_type&;
    using pointer         = value_type*;
    using const_pointer   = const value_type*;

    using iterator       = value_type*;
    using const_iterator = const value_type*;

    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // Element access

    [[nodiscard]] constexpr reference at(size_type pos) {
#ifndef CHESS_NO_EXCEPTIONS
        if (pos >= size_) {
            throw std::out_of_range("Movelist::at: pos (which is " + std::to_string(pos) + ") >= size (which is " +
                                    std::to_string(size_) + ")");
        }
#endif
        return moves_[pos];
    }

    [[nodiscard]] constexpr const_reference at(size_type pos) const {
#ifndef CHESS_NO_EXCEPTIONS
        if (pos >= size_) {
            throw std::out_of_range("Movelist::at: pos (which is " + std::to_string(pos) + ") >= size (which is " +
                                    std::to_string(size_) + ")");
        }
#endif
        return moves_[pos];
    }

    [[nodiscard]] constexpr reference operator[](size_type pos) noexcept { return moves_[pos]; }
    [[nodiscard]] constexpr const_reference operator[](size_type pos) const noexcept { return moves_[pos]; }

    [[nodiscard]] constexpr reference front() noexcept { return moves_[0]; }
    [[nodiscard]] constexpr const_reference front() const noexcept { return moves_[0]; }

    [[nodiscard]] constexpr reference back() noexcept { return moves_[size_ - 1]; }
    [[nodiscard]] constexpr const_reference back() const noexcept { return moves_[size_ - 1]; }

    // Iterators

    [[nodiscard]] constexpr iterator begin() noexcept { return &moves_[0]; }
    [[nodiscard]] constexpr const_iterator begin() const noexcept { return &moves_[0]; }

    [[nodiscard]] constexpr iterator end() noexcept { return &moves_[0] + size_; }
    [[nodiscard]] constexpr const_iterator end() const noexcept { return &moves_[0] + size_; }

    // Capacity

    /**
     * @brief Checks if the movelist is empty.
     * @return
     */
    [[nodiscard]] constexpr bool empty() const noexcept { return size_ == 0; }

    /**
     * @brief Return the number of moves in the movelist.
     * @return
     */
    [[nodiscard]] constexpr size_type size() const noexcept { return size_; }

    // Modifiers

    /**
     * @brief Clears the movelist.
     */
    constexpr void clear() noexcept { size_ = 0; }

    /**
     * @brief Add a move to the end of the movelist.
     * @param move
     */
    constexpr void add(const_reference move) noexcept {
        assert(size_ < constants::MAX_MOVES);
        moves_[size_++] = move;
    }

    /**
     * @brief Add a move to the end of the movelist.
     * @param move
     */
    constexpr void add(value_type&& move) noexcept {
        assert(size_ < constants::MAX_MOVES);
        moves_[size_++] = move;
    }

    // Other

    /**
     * @brief Checks if a move is in the movelist, returns the index of the move if it is found, otherwise -1.
     * @param move
     * @return
     */
    [[nodiscard]] [[deprecated("Use std::find() instead.")]] constexpr size_type find(value_type move) const noexcept {
        for (size_type i = 0; i < size_; ++i) {
            if (moves_[i] == move) {
                return i;
            }
        }

        return -1;
    }

   private:
    std::array<value_type, constants::MAX_MOVES> moves_;
    size_type size_ = 0;
};
}  // namespace chess

namespace chess {
enum PieceGenType {
    PAWN   = 1,
    KNIGHT = 2,
    BISHOP = 4,
    ROOK   = 8,
    QUEEN  = 16,
    KING   = 32,
};

class Board;

class movegen {
   public:
    enum class MoveGenType : std::uint8_t { ALL, CAPTURE, QUIET };

    /**
     * @brief Generates all legal moves for a position.
     * @tparam mt
     * @param movelist
     * @param board
     * @param pieces
     */
    template <MoveGenType mt = MoveGenType::ALL>
    void static legalmoves(Movelist &movelist, const Board &board,
                           int pieces = PieceGenType::PAWN | PieceGenType::KNIGHT | PieceGenType::BISHOP |
                                        PieceGenType::ROOK | PieceGenType::QUEEN | PieceGenType::KING);

   private:
    static auto init_squares_between();
    static const std::array<std::array<Bitboard, 64>, 64> SQUARES_BETWEEN_BB;

    // Generate the checkmask. Returns a bitboard where the attacker path between the king and enemy piece is set.
    template <Color::underlying c>
    [[nodiscard]] static std::pair<Bitboard, int> checkMask(const Board &board, Square sq);

    // Generate the pin mask for horizontal and vertical pins -> PieceType::ROOK
    // Generate the pin mask for diagonal pins. -> PieceType::BISHOP
    // Returns a bitboard where the ray between the king and the pinner is set.
    template <Color::underlying c, PieceType::underlying pt>
    [[nodiscard]] static Bitboard pinMask(const Board &board, Square sq, Bitboard occ_enemy, Bitboard occ_us) noexcept;

    // Returns the squares that are attacked by the enemy
    template <Color::underlying c>
    [[nodiscard]] static Bitboard seenSquares(const Board &board, Bitboard enemy_empty);

    // Generate pawn moves.
    template <Color::underlying c, MoveGenType mt>
    static void generatePawnMoves(const Board &board, Movelist &moves, Bitboard pin_d, Bitboard pin_hv,
                                  Bitboard checkmask, Bitboard occ_enemy);

    [[nodiscard]] static std::array<Move, 2> generateEPMove(const Board &board, Bitboard checkmask, Bitboard pin_d,
                                                            Bitboard pawns_lr, Square ep, Color c);

    [[nodiscard]] static Bitboard generateKnightMoves(Square sq);

    [[nodiscard]] static Bitboard generateBishopMoves(Square sq, Bitboard pin_d, Bitboard occ_all);

    [[nodiscard]] static Bitboard generateRookMoves(Square sq, Bitboard pin_hv, Bitboard occ_all);

    [[nodiscard]] static Bitboard generateQueenMoves(Square sq, Bitboard pin_d, Bitboard pin_hv, Bitboard occ_all);

    [[nodiscard]] static Bitboard generateKingMoves(Square sq, Bitboard seen, Bitboard movable_square);

    template <Color::underlying c>
    [[nodiscard]] static Bitboard generateCastleMoves(const Board &board, Square sq, Bitboard seen, Bitboard pinHV) noexcept;

    template <typename T>
    static void whileBitboardAdd(Movelist &movelist, Bitboard mask, T func);

    template <Color::underlying c, MoveGenType mt>
    static void legalmoves(Movelist &movelist, const Board &board, int pieces);

    template <Color::underlying c>
    static bool isEpSquareValid(const Board &board, Square ep);

    [[nodiscard]] static Bitboard between(Square sq1, Square sq2) noexcept;

    friend class Board;
};

}  // namespace chess



namespace chess {
class Zobrist {
    using U64                              = std::uint64_t;
    static constexpr U64 RANDOM_ARRAY[781] = {
        0x9D39247E33776D41, 0x2AF7398005AAA5C7, 0x44DB015024623547, 0x9C15F73E62A76AE2, 0x75834465489C0C89,
        0x3290AC3A203001BF, 0x0FBBAD1F61042279, 0xE83A908FF2FB60CA, 0x0D7E765D58755C10, 0x1A083822CEAFE02D,
        0x9605D5F0E25EC3B0, 0xD021FF5CD13A2ED5, 0x40BDF15D4A672E32, 0x011355146FD56395, 0x5DB4832046F3D9E5,
        0x239F8B2D7FF719CC, 0x05D1A1AE85B49AA1, 0x679F848F6E8FC971, 0x7449BBFF801FED0B, 0x7D11CDB1C3B7ADF0,
        0x82C7709E781EB7CC, 0xF3218F1C9510786C, 0x331478F3AF51BBE6, 0x4BB38DE5E7219443, 0xAA649C6EBCFD50FC,
        0x8DBD98A352AFD40B, 0x87D2074B81D79217, 0x19F3C751D3E92AE1, 0xB4AB30F062B19ABF, 0x7B0500AC42047AC4,
        0xC9452CA81A09D85D, 0x24AA6C514DA27500, 0x4C9F34427501B447, 0x14A68FD73C910841, 0xA71B9B83461CBD93,
        0x03488B95B0F1850F, 0x637B2B34FF93C040, 0x09D1BC9A3DD90A94, 0x3575668334A1DD3B, 0x735E2B97A4C45A23,
        0x18727070F1BD400B, 0x1FCBACD259BF02E7, 0xD310A7C2CE9B6555, 0xBF983FE0FE5D8244, 0x9F74D14F7454A824,
        0x51EBDC4AB9BA3035, 0x5C82C505DB9AB0FA, 0xFCF7FE8A3430B241, 0x3253A729B9BA3DDE, 0x8C74C368081B3075,
        0xB9BC6C87167C33E7, 0x7EF48F2B83024E20, 0x11D505D4C351BD7F, 0x6568FCA92C76A243, 0x4DE0B0F40F32A7B8,
        0x96D693460CC37E5D, 0x42E240CB63689F2F, 0x6D2BDCDAE2919661, 0x42880B0236E4D951, 0x5F0F4A5898171BB6,
        0x39F890F579F92F88, 0x93C5B5F47356388B, 0x63DC359D8D231B78, 0xEC16CA8AEA98AD76, 0x5355F900C2A82DC7,
        0x07FB9F855A997142, 0x5093417AA8A7ED5E, 0x7BCBC38DA25A7F3C, 0x19FC8A768CF4B6D4, 0x637A7780DECFC0D9,
        0x8249A47AEE0E41F7, 0x79AD695501E7D1E8, 0x14ACBAF4777D5776, 0xF145B6BECCDEA195, 0xDABF2AC8201752FC,
        0x24C3C94DF9C8D3F6, 0xBB6E2924F03912EA, 0x0CE26C0B95C980D9, 0xA49CD132BFBF7CC4, 0xE99D662AF4243939,
        0x27E6AD7891165C3F, 0x8535F040B9744FF1, 0x54B3F4FA5F40D873, 0x72B12C32127FED2B, 0xEE954D3C7B411F47,
        0x9A85AC909A24EAA1, 0x70AC4CD9F04F21F5, 0xF9B89D3E99A075C2, 0x87B3E2B2B5C907B1, 0xA366E5B8C54F48B8,
        0xAE4A9346CC3F7CF2, 0x1920C04D47267BBD, 0x87BF02C6B49E2AE9, 0x092237AC237F3859, 0xFF07F64EF8ED14D0,
        0x8DE8DCA9F03CC54E, 0x9C1633264DB49C89, 0xB3F22C3D0B0B38ED, 0x390E5FB44D01144B, 0x5BFEA5B4712768E9,
        0x1E1032911FA78984, 0x9A74ACB964E78CB3, 0x4F80F7A035DAFB04, 0x6304D09A0B3738C4, 0x2171E64683023A08,
        0x5B9B63EB9CEFF80C, 0x506AACF489889342, 0x1881AFC9A3A701D6, 0x6503080440750644, 0xDFD395339CDBF4A7,
        0xEF927DBCF00C20F2, 0x7B32F7D1E03680EC, 0xB9FD7620E7316243, 0x05A7E8A57DB91B77, 0xB5889C6E15630A75,
        0x4A750A09CE9573F7, 0xCF464CEC899A2F8A, 0xF538639CE705B824, 0x3C79A0FF5580EF7F, 0xEDE6C87F8477609D,
        0x799E81F05BC93F31, 0x86536B8CF3428A8C, 0x97D7374C60087B73, 0xA246637CFF328532, 0x043FCAE60CC0EBA0,
        0x920E449535DD359E, 0x70EB093B15B290CC, 0x73A1921916591CBD, 0x56436C9FE1A1AA8D, 0xEFAC4B70633B8F81,
        0xBB215798D45DF7AF, 0x45F20042F24F1768, 0x930F80F4E8EB7462, 0xFF6712FFCFD75EA1, 0xAE623FD67468AA70,
        0xDD2C5BC84BC8D8FC, 0x7EED120D54CF2DD9, 0x22FE545401165F1C, 0xC91800E98FB99929, 0x808BD68E6AC10365,
        0xDEC468145B7605F6, 0x1BEDE3A3AEF53302, 0x43539603D6C55602, 0xAA969B5C691CCB7A, 0xA87832D392EFEE56,
        0x65942C7B3C7E11AE, 0xDED2D633CAD004F6, 0x21F08570F420E565, 0xB415938D7DA94E3C, 0x91B859E59ECB6350,
        0x10CFF333E0ED804A, 0x28AED140BE0BB7DD, 0xC5CC1D89724FA456, 0x5648F680F11A2741, 0x2D255069F0B7DAB3,
        0x9BC5A38EF729ABD4, 0xEF2F054308F6A2BC, 0xAF2042F5CC5C2858, 0x480412BAB7F5BE2A, 0xAEF3AF4A563DFE43,
        0x19AFE59AE451497F, 0x52593803DFF1E840, 0xF4F076E65F2CE6F0, 0x11379625747D5AF3, 0xBCE5D2248682C115,
        0x9DA4243DE836994F, 0x066F70B33FE09017, 0x4DC4DE189B671A1C, 0x51039AB7712457C3, 0xC07A3F80C31FB4B4,
        0xB46EE9C5E64A6E7C, 0xB3819A42ABE61C87, 0x21A007933A522A20, 0x2DF16F761598AA4F, 0x763C4A1371B368FD,
        0xF793C46702E086A0, 0xD7288E012AEB8D31, 0xDE336A2A4BC1C44B, 0x0BF692B38D079F23, 0x2C604A7A177326B3,
        0x4850E73E03EB6064, 0xCFC447F1E53C8E1B, 0xB05CA3F564268D99, 0x9AE182C8BC9474E8, 0xA4FC4BD4FC5558CA,
        0xE755178D58FC4E76, 0x69B97DB1A4C03DFE, 0xF9B5B7C4ACC67C96, 0xFC6A82D64B8655FB, 0x9C684CB6C4D24417,
        0x8EC97D2917456ED0, 0x6703DF9D2924E97E, 0xC547F57E42A7444E, 0x78E37644E7CAD29E, 0xFE9A44E9362F05FA,
        0x08BD35CC38336615, 0x9315E5EB3A129ACE, 0x94061B871E04DF75, 0xDF1D9F9D784BA010, 0x3BBA57B68871B59D,
        0xD2B7ADEEDED1F73F, 0xF7A255D83BC373F8, 0xD7F4F2448C0CEB81, 0xD95BE88CD210FFA7, 0x336F52F8FF4728E7,
        0xA74049DAC312AC71, 0xA2F61BB6E437FDB5, 0x4F2A5CB07F6A35B3, 0x87D380BDA5BF7859, 0x16B9F7E06C453A21,
        0x7BA2484C8A0FD54E, 0xF3A678CAD9A2E38C, 0x39B0BF7DDE437BA2, 0xFCAF55C1BF8A4424, 0x18FCF680573FA594,
        0x4C0563B89F495AC3, 0x40E087931A00930D, 0x8CFFA9412EB642C1, 0x68CA39053261169F, 0x7A1EE967D27579E2,
        0x9D1D60E5076F5B6F, 0x3810E399B6F65BA2, 0x32095B6D4AB5F9B1, 0x35CAB62109DD038A, 0xA90B24499FCFAFB1,
        0x77A225A07CC2C6BD, 0x513E5E634C70E331, 0x4361C0CA3F692F12, 0xD941ACA44B20A45B, 0x528F7C8602C5807B,
        0x52AB92BEB9613989, 0x9D1DFA2EFC557F73, 0x722FF175F572C348, 0x1D1260A51107FE97, 0x7A249A57EC0C9BA2,
        0x04208FE9E8F7F2D6, 0x5A110C6058B920A0, 0x0CD9A497658A5698, 0x56FD23C8F9715A4C, 0x284C847B9D887AAE,
        0x04FEABFBBDB619CB, 0x742E1E651C60BA83, 0x9A9632E65904AD3C, 0x881B82A13B51B9E2, 0x506E6744CD974924,
        0xB0183DB56FFC6A79, 0x0ED9B915C66ED37E, 0x5E11E86D5873D484, 0xF678647E3519AC6E, 0x1B85D488D0F20CC5,
        0xDAB9FE6525D89021, 0x0D151D86ADB73615, 0xA865A54EDCC0F019, 0x93C42566AEF98FFB, 0x99E7AFEABE000731,
        0x48CBFF086DDF285A, 0x7F9B6AF1EBF78BAF, 0x58627E1A149BBA21, 0x2CD16E2ABD791E33, 0xD363EFF5F0977996,
        0x0CE2A38C344A6EED, 0x1A804AADB9CFA741, 0x907F30421D78C5DE, 0x501F65EDB3034D07, 0x37624AE5A48FA6E9,
        0x957BAF61700CFF4E, 0x3A6C27934E31188A, 0xD49503536ABCA345, 0x088E049589C432E0, 0xF943AEE7FEBF21B8,
        0x6C3B8E3E336139D3, 0x364F6FFA464EE52E, 0xD60F6DCEDC314222, 0x56963B0DCA418FC0, 0x16F50EDF91E513AF,
        0xEF1955914B609F93, 0x565601C0364E3228, 0xECB53939887E8175, 0xBAC7A9A18531294B, 0xB344C470397BBA52,
        0x65D34954DAF3CEBD, 0xB4B81B3FA97511E2, 0xB422061193D6F6A7, 0x071582401C38434D, 0x7A13F18BBEDC4FF5,
        0xBC4097B116C524D2, 0x59B97885E2F2EA28, 0x99170A5DC3115544, 0x6F423357E7C6A9F9, 0x325928EE6E6F8794,
        0xD0E4366228B03343, 0x565C31F7DE89EA27, 0x30F5611484119414, 0xD873DB391292ED4F, 0x7BD94E1D8E17DEBC,
        0xC7D9F16864A76E94, 0x947AE053EE56E63C, 0xC8C93882F9475F5F, 0x3A9BF55BA91F81CA, 0xD9A11FBB3D9808E4,
        0x0FD22063EDC29FCA, 0xB3F256D8ACA0B0B9, 0xB03031A8B4516E84, 0x35DD37D5871448AF, 0xE9F6082B05542E4E,
        0xEBFAFA33D7254B59, 0x9255ABB50D532280, 0xB9AB4CE57F2D34F3, 0x693501D628297551, 0xC62C58F97DD949BF,
        0xCD454F8F19C5126A, 0xBBE83F4ECC2BDECB, 0xDC842B7E2819E230, 0xBA89142E007503B8, 0xA3BC941D0A5061CB,
        0xE9F6760E32CD8021, 0x09C7E552BC76492F, 0x852F54934DA55CC9, 0x8107FCCF064FCF56, 0x098954D51FFF6580,
        0x23B70EDB1955C4BF, 0xC330DE426430F69D, 0x4715ED43E8A45C0A, 0xA8D7E4DAB780A08D, 0x0572B974F03CE0BB,
        0xB57D2E985E1419C7, 0xE8D9ECBE2CF3D73F, 0x2FE4B17170E59750, 0x11317BA87905E790, 0x7FBF21EC8A1F45EC,
        0x1725CABFCB045B00, 0x964E915CD5E2B207, 0x3E2B8BCBF016D66D, 0xBE7444E39328A0AC, 0xF85B2B4FBCDE44B7,
        0x49353FEA39BA63B1, 0x1DD01AAFCD53486A, 0x1FCA8A92FD719F85, 0xFC7C95D827357AFA, 0x18A6A990C8B35EBD,
        0xCCCB7005C6B9C28D, 0x3BDBB92C43B17F26, 0xAA70B5B4F89695A2, 0xE94C39A54A98307F, 0xB7A0B174CFF6F36E,
        0xD4DBA84729AF48AD, 0x2E18BC1AD9704A68, 0x2DE0966DAF2F8B1C, 0xB9C11D5B1E43A07E, 0x64972D68DEE33360,
        0x94628D38D0C20584, 0xDBC0D2B6AB90A559, 0xD2733C4335C6A72F, 0x7E75D99D94A70F4D, 0x6CED1983376FA72B,
        0x97FCAACBF030BC24, 0x7B77497B32503B12, 0x8547EDDFB81CCB94, 0x79999CDFF70902CB, 0xCFFE1939438E9B24,
        0x829626E3892D95D7, 0x92FAE24291F2B3F1, 0x63E22C147B9C3403, 0xC678B6D860284A1C, 0x5873888850659AE7,
        0x0981DCD296A8736D, 0x9F65789A6509A440, 0x9FF38FED72E9052F, 0xE479EE5B9930578C, 0xE7F28ECD2D49EECD,
        0x56C074A581EA17FE, 0x5544F7D774B14AEF, 0x7B3F0195FC6F290F, 0x12153635B2C0CF57, 0x7F5126DBBA5E0CA7,
        0x7A76956C3EAFB413, 0x3D5774A11D31AB39, 0x8A1B083821F40CB4, 0x7B4A38E32537DF62, 0x950113646D1D6E03,
        0x4DA8979A0041E8A9, 0x3BC36E078F7515D7, 0x5D0A12F27AD310D1, 0x7F9D1A2E1EBE1327, 0xDA3A361B1C5157B1,
        0xDCDD7D20903D0C25, 0x36833336D068F707, 0xCE68341F79893389, 0xAB9090168DD05F34, 0x43954B3252DC25E5,
        0xB438C2B67F98E5E9, 0x10DCD78E3851A492, 0xDBC27AB5447822BF, 0x9B3CDB65F82CA382, 0xB67B7896167B4C84,
        0xBFCED1B0048EAC50, 0xA9119B60369FFEBD, 0x1FFF7AC80904BF45, 0xAC12FB171817EEE7, 0xAF08DA9177DDA93D,
        0x1B0CAB936E65C744, 0xB559EB1D04E5E932, 0xC37B45B3F8D6F2BA, 0xC3A9DC228CAAC9E9, 0xF3B8B6675A6507FF,
        0x9FC477DE4ED681DA, 0x67378D8ECCEF96CB, 0x6DD856D94D259236, 0xA319CE15B0B4DB31, 0x073973751F12DD5E,
        0x8A8E849EB32781A5, 0xE1925C71285279F5, 0x74C04BF1790C0EFE, 0x4DDA48153C94938A, 0x9D266D6A1CC0542C,
        0x7440FB816508C4FE, 0x13328503DF48229F, 0xD6BF7BAEE43CAC40, 0x4838D65F6EF6748F, 0x1E152328F3318DEA,
        0x8F8419A348F296BF, 0x72C8834A5957B511, 0xD7A023A73260B45C, 0x94EBC8ABCFB56DAE, 0x9FC10D0F989993E0,
        0xDE68A2355B93CAE6, 0xA44CFE79AE538BBE, 0x9D1D84FCCE371425, 0x51D2B1AB2DDFB636, 0x2FD7E4B9E72CD38C,
        0x65CA5B96B7552210, 0xDD69A0D8AB3B546D, 0x604D51B25FBF70E2, 0x73AA8A564FB7AC9E, 0x1A8C1E992B941148,
        0xAAC40A2703D9BEA0, 0x764DBEAE7FA4F3A6, 0x1E99B96E70A9BE8B, 0x2C5E9DEB57EF4743, 0x3A938FEE32D29981,
        0x26E6DB8FFDF5ADFE, 0x469356C504EC9F9D, 0xC8763C5B08D1908C, 0x3F6C6AF859D80055, 0x7F7CC39420A3A545,
        0x9BFB227EBDF4C5CE, 0x89039D79D6FC5C5C, 0x8FE88B57305E2AB6, 0xA09E8C8C35AB96DE, 0xFA7E393983325753,
        0xD6B6D0ECC617C699, 0xDFEA21EA9E7557E3, 0xB67C1FA481680AF8, 0xCA1E3785A9E724E5, 0x1CFC8BED0D681639,
        0xD18D8549D140CAEA, 0x4ED0FE7E9DC91335, 0xE4DBF0634473F5D2, 0x1761F93A44D5AEFE, 0x53898E4C3910DA55,
        0x734DE8181F6EC39A, 0x2680B122BAA28D97, 0x298AF231C85BAFAB, 0x7983EED3740847D5, 0x66C1A2A1A60CD889,
        0x9E17E49642A3E4C1, 0xEDB454E7BADC0805, 0x50B704CAB602C329, 0x4CC317FB9CDDD023, 0x66B4835D9EAFEA22,
        0x219B97E26FFC81BD, 0x261E4E4C0A333A9D, 0x1FE2CCA76517DB90, 0xD7504DFA8816EDBB, 0xB9571FA04DC089C8,
        0x1DDC0325259B27DE, 0xCF3F4688801EB9AA, 0xF4F5D05C10CAB243, 0x38B6525C21A42B0E, 0x36F60E2BA4FA6800,
        0xEB3593803173E0CE, 0x9C4CD6257C5A3603, 0xAF0C317D32ADAA8A, 0x258E5A80C7204C4B, 0x8B889D624D44885D,
        0xF4D14597E660F855, 0xD4347F66EC8941C3, 0xE699ED85B0DFB40D, 0x2472F6207C2D0484, 0xC2A1E7B5B459AEB5,
        0xAB4F6451CC1D45EC, 0x63767572AE3D6174, 0xA59E0BD101731A28, 0x116D0016CB948F09, 0x2CF9C8CA052F6E9F,
        0x0B090A7560A968E3, 0xABEEDDB2DDE06FF1, 0x58EFC10B06A2068D, 0xC6E57A78FBD986E0, 0x2EAB8CA63CE802D7,
        0x14A195640116F336, 0x7C0828DD624EC390, 0xD74BBE77E6116AC7, 0x804456AF10F5FB53, 0xEBE9EA2ADF4321C7,
        0x03219A39EE587A30, 0x49787FEF17AF9924, 0xA1E9300CD8520548, 0x5B45E522E4B1B4EF, 0xB49C3B3995091A36,
        0xD4490AD526F14431, 0x12A8F216AF9418C2, 0x001F837CC7350524, 0x1877B51E57A764D5, 0xA2853B80F17F58EE,
        0x993E1DE72D36D310, 0xB3598080CE64A656, 0x252F59CF0D9F04BB, 0xD23C8E176D113600, 0x1BDA0492E7E4586E,
        0x21E0BD5026C619BF, 0x3B097ADAF088F94E, 0x8D14DEDB30BE846E, 0xF95CFFA23AF5F6F4, 0x3871700761B3F743,
        0xCA672B91E9E4FA16, 0x64C8E531BFF53B55, 0x241260ED4AD1E87D, 0x106C09B972D2E822, 0x7FBA195410E5CA30,
        0x7884D9BC6CB569D8, 0x0647DFEDCD894A29, 0x63573FF03E224774, 0x4FC8E9560F91B123, 0x1DB956E450275779,
        0xB8D91274B9E9D4FB, 0xA2EBEE47E2FBFCE1, 0xD9F1F30CCD97FB09, 0xEFED53D75FD64E6B, 0x2E6D02C36017F67F,
        0xA9AA4D20DB084E9B, 0xB64BE8D8B25396C1, 0x70CB6AF7C2D5BCF0, 0x98F076A4F7A2322E, 0xBF84470805E69B5F,
        0x94C3251F06F90CF3, 0x3E003E616A6591E9, 0xB925A6CD0421AFF3, 0x61BDD1307C66E300, 0xBF8D5108E27E0D48,
        0x240AB57A8B888B20, 0xFC87614BAF287E07, 0xEF02CDD06FFDB432, 0xA1082C0466DF6C0A, 0x8215E577001332C8,
        0xD39BB9C3A48DB6CF, 0x2738259634305C14, 0x61CF4F94C97DF93D, 0x1B6BACA2AE4E125B, 0x758F450C88572E0B,
        0x959F587D507A8359, 0xB063E962E045F54D, 0x60E8ED72C0DFF5D1, 0x7B64978555326F9F, 0xFD080D236DA814BA,
        0x8C90FD9B083F4558, 0x106F72FE81E2C590, 0x7976033A39F7D952, 0xA4EC0132764CA04B, 0x733EA705FAE4FA77,
        0xB4D8F77BC3E56167, 0x9E21F4F903B33FD9, 0x9D765E419FB69F6D, 0xD30C088BA61EA5EF, 0x5D94337FBFAF7F5B,
        0x1A4E4822EB4D7A59, 0x6FFE73E81B637FB3, 0xDDF957BC36D8B9CA, 0x64D0E29EEA8838B3, 0x08DD9BDFD96B9F63,
        0x087E79E5A57D1D13, 0xE328E230E3E2B3FB, 0x1C2559E30F0946BE, 0x720BF5F26F4D2EAA, 0xB0774D261CC609DB,
        0x443F64EC5A371195, 0x4112CF68649A260E, 0xD813F2FAB7F5C5CA, 0x660D3257380841EE, 0x59AC2C7873F910A3,
        0xE846963877671A17, 0x93B633ABFA3469F8, 0xC0C0F5A60EF4CDCF, 0xCAF21ECD4377B28C, 0x57277707199B8175,
        0x506C11B9D90E8B1D, 0xD83CC2687A19255F, 0x4A29C6465A314CD1, 0xED2DF21216235097, 0xB5635C95FF7296E2,
        0x22AF003AB672E811, 0x52E762596BF68235, 0x9AEBA33AC6ECC6B0, 0x944F6DE09134DFB6, 0x6C47BEC883A7DE39,
        0x6AD047C430A12104, 0xA5B1CFDBA0AB4067, 0x7C45D833AFF07862, 0x5092EF950A16DA0B, 0x9338E69C052B8E7B,
        0x455A4B4CFE30E3F5, 0x6B02E63195AD0CF8, 0x6B17B224BAD6BF27, 0xD1E0CCD25BB9C169, 0xDE0C89A556B9AE70,
        0x50065E535A213CF6, 0x9C1169FA2777B874, 0x78EDEFD694AF1EED, 0x6DC93D9526A50E68, 0xEE97F453F06791ED,
        0x32AB0EDB696703D3, 0x3A6853C7E70757A7, 0x31865CED6120F37D, 0x67FEF95D92607890, 0x1F2B1D1F15F6DC9C,
        0xB69E38A8965C6B65, 0xAA9119FF184CCCF4, 0xF43C732873F24C13, 0xFB4A3D794A9A80D2, 0x3550C2321FD6109C,
        0x371F77E76BB8417E, 0x6BFA9AAE5EC05779, 0xCD04F3FF001A4778, 0xE3273522064480CA, 0x9F91508BFFCFC14A,
        0x049A7F41061A9E60, 0xFCB6BE43A9F2FE9B, 0x08DE8A1C7797DA9B, 0x8F9887E6078735A1, 0xB5B4071DBFC73A66,
        0x230E343DFBA08D33, 0x43ED7F5A0FAE657D, 0x3A88A0FBBCB05C63, 0x21874B8B4D2DBC4F, 0x1BDEA12E35F6A8C9,
        0x53C065C6C8E63528, 0xE34A1D250E7A8D6B, 0xD6B04D3B7651DD7E, 0x5E90277E7CB39E2D, 0x2C046F22062DC67D,
        0xB10BB459132D0A26, 0x3FA9DDFB67E2F199, 0x0E09B88E1914F7AF, 0x10E8B35AF3EEAB37, 0x9EEDECA8E272B933,
        0xD4C718BC4AE8AE5F, 0x81536D601170FC20, 0x91B534F885818A06, 0xEC8177F83F900978, 0x190E714FADA5156E,
        0xB592BF39B0364963, 0x89C350C893AE7DC1, 0xAC042E70F8B383F2, 0xB49B52E587A1EE60, 0xFB152FE3FF26DA89,
        0x3E666E6F69AE2C15, 0x3B544EBE544C19F9, 0xE805A1E290CF2456, 0x24B33C9D7ED25117, 0xE74733427B72F0C1,
        0x0A804D18B7097475, 0x57E3306D881EDB4F, 0x4AE7D6A36EB5DBCB, 0x2D8D5432157064C8, 0xD1E649DE1E7F268B,
        0x8A328A1CEDFE552C, 0x07A3AEC79624C7DA, 0x84547DDC3E203C94, 0x990A98FD5071D263, 0x1A4FF12616EEFC89,
        0xF6F7FD1431714200, 0x30C05B1BA332F41C, 0x8D2636B81555A786, 0x46C9FEB55D120902, 0xCCEC0A73B49C9921,
        0x4E9D2827355FC492, 0x19EBB029435DCB0F, 0x4659D2B743848A2C, 0x963EF2C96B33BE31, 0x74F85198B05A2E7D,
        0x5A0F544DD2B1FB18, 0x03727073C2E134B1, 0xC7F6AA2DE59AEA61, 0x352787BAA0D7C22F, 0x9853EAB63B5E0B35,
        0xABBDCDD7ED5C0860, 0xCF05DAF5AC8D77B0, 0x49CAD48CEBF4A71E, 0x7A4C10EC2158C4A6, 0xD9E92AA246BF719E,
        0x13AE978D09FE5557, 0x730499AF921549FF, 0x4E4B705B92903BA4, 0xFF577222C14F0A3A, 0x55B6344CF97AAFAE,
        0xB862225B055B6960, 0xCAC09AFBDDD2CDB4, 0xDAF8E9829FE96B5F, 0xB5FDFC5D3132C498, 0x310CB380DB6F7503,
        0xE87FBB46217A360E, 0x2102AE466EBB1148, 0xF8549E1A3AA5E00D, 0x07A69AFDCC42261A, 0xC4C118BFE78FEAAE,
        0xF9F4892ED96BD438, 0x1AF3DBE25D8F45DA, 0xF5B4B0B0D2DEEEB4, 0x962ACEEFA82E1C84, 0x046E3ECAAF453CE9,
        0xF05D129681949A4C, 0x964781CE734B3C84, 0x9C2ED44081CE5FBD, 0x522E23F3925E319E, 0x177E00F9FC32F791,
        0x2BC60A63A6F3B3F2, 0x222BBFAE61725606, 0x486289DDCC3D6780, 0x7DC7785B8EFDFC80, 0x8AF38731C02BA980,
        0x1FAB64EA29A2DDF7, 0xE4D9429322CD065A, 0x9DA058C67844F20C, 0x24C0E332B70019B0, 0x233003B5A6CFE6AD,
        0xD586BD01C5C217F6, 0x5E5637885F29BC2B, 0x7EBA726D8C94094B, 0x0A56A5F0BFE39272, 0xD79476A84EE20D06,
        0x9E4C1269BAA4BF37, 0x17EFEE45B0DEE640, 0x1D95B0A5FCF90BC6, 0x93CBE0B699C2585D, 0x65FA4F227A2B6D79,
        0xD5F9E858292504D5, 0xC2B5A03F71471A6F, 0x59300222B4561E00, 0xCE2F8642CA0712DC, 0x7CA9723FBB2E8988,
        0x2785338347F2BA08, 0xC61BB3A141E50E8C, 0x150F361DAB9DEC26, 0x9F6A419D382595F4, 0x64A53DC924FE7AC9,
        0x142DE49FFF7A7C3D, 0x0C335248857FA9E7, 0x0A9C32D5EAE45305, 0xE6C42178C4BBB92E, 0x71F1CE2490D20B07,
        0xF1BCC3D275AFE51A, 0xE728E8C83C334074, 0x96FBF83A12884624, 0x81A1549FD6573DA5, 0x5FA7867CAF35E149,
        0x56986E2EF3ED091B, 0x917F1DD5F8886C61, 0xD20D8C88C8FFE65F, 0x31D71DCE64B2C310, 0xF165B587DF898190,
        0xA57E6339DD2CF3A0, 0x1EF6E6DBB1961EC9, 0x70CC73D90BC26E24, 0xE21A6B35DF0C3AD7, 0x003A93D8B2806962,
        0x1C99DED33CB890A1, 0xCF3145DE0ADD4289, 0xD0E4427A5514FB72, 0x77C621CC9FB3A483, 0x67A34DAC4356550B,
        0xF8D626AAAF278509};

    static constexpr std::array<U64, 16> castlingKey = []() constexpr {
        auto generateCastlingKey = [](int index) constexpr -> U64 {
            constexpr int RANDOM_OFFSET = 768;
            constexpr int RANDOM_COUNT  = 4;

            U64 key = 0;

            for (int i = 0; i < RANDOM_COUNT; ++i) {
                if (index & (1 << i)) {
                    key ^= RANDOM_ARRAY[RANDOM_OFFSET + i];
                }
            }

            return key;
        };

        std::array<U64, 16> arr{};

        for (int i = 0; i < 16; ++i) arr[i] = generateCastlingKey(i);

        return arr;
    }();

    static constexpr int MAP_HASH_PIECE[12] = {1, 3, 5, 7, 9, 11, 0, 2, 4, 6, 8, 10};

    [[nodiscard]] static U64 piece(Piece piece, Square square) noexcept {
        assert(piece < 12);
        return RANDOM_ARRAY[64 * MAP_HASH_PIECE[piece] + square.index()];
    }

    [[nodiscard]] static U64 enpassant(File file) noexcept {
        assert(static_cast<int>(file) < 8);
        return RANDOM_ARRAY[772 + file];
    }

    [[nodiscard]] static U64 castling(int castling) noexcept {
        assert(castling >= 0 && castling < 16);
        return castlingKey[castling];
    }

    [[nodiscard]] static U64 castlingIndex(int idx) noexcept {
        assert(idx >= 0 && idx < 4);
        return RANDOM_ARRAY[768 + idx];
    }

    [[nodiscard]] static U64 sideToMove() noexcept { return RANDOM_ARRAY[780]; }

   public:
    friend class Board;
};

}  // namespace chess

namespace chess {

namespace detail {
inline std::optional<int> parseStringViewToInt(std::string_view sv) {
    if (sv.empty()) return std::nullopt;

    std::string_view parsed_sv = sv;
    if (parsed_sv.back() == ';') parsed_sv.remove_suffix(1);

    if (parsed_sv.empty()) return std::nullopt;

#ifdef CHESS_USE_CHARCONV
    int result;
    const char *begin = parsed_sv.data();
    const char *end   = begin + parsed_sv.size();

    auto [ptr, ec] = std::from_chars(begin, end, result);

    if (ec == std::errc() && ptr == end) {
        return result;
    }
#else
    std::string str(parsed_sv);
    std::stringstream ss(str);

    ss.exceptions(std::ios::goodbit);

    int value;
    ss >> value;

    if (!ss.fail() && (ss.eof() || (ss >> std::ws).eof())) {
        return value;
    }
#endif

    return std::nullopt;
}
}  // namespace detail

enum class GameResult { WIN, LOSE, DRAW, NONE };

enum class GameResultReason {
    CHECKMATE,
    STALEMATE,
    INSUFFICIENT_MATERIAL,
    FIFTY_MOVE_RULE,
    THREEFOLD_REPETITION,
    NONE
};

enum class CheckType { NO_CHECK, DIRECT_CHECK, DISCOVERY_CHECK };

// A compact representation of the board in 24 bytes,
// does not include the half-move clock or full move number.
using PackedBoard = std::array<std::uint8_t, 24>;

class Board {
    using U64 = std::uint64_t;

   public:
    class CastlingRights {
       public:
        enum class Side : std::uint8_t { KING_SIDE, QUEEN_SIDE };

        constexpr void setCastlingRight(Color color, Side castle, File rook_file) {
            rooks[color][static_cast<int>(castle)] = rook_file;
        }

        constexpr void clear() { rooks[0][0] = rooks[0][1] = rooks[1][0] = rooks[1][1] = File::NO_FILE; }

        constexpr int clear(Color color, Side castle) {
            rooks[color][static_cast<int>(castle)] = File::NO_FILE;
            return color * 2 + static_cast<int>(castle);
        }

        constexpr void clear(Color color) { rooks[color][0] = rooks[color][1] = File::NO_FILE; }

        constexpr bool has(Color color, Side castle) const {
            return rooks[color][static_cast<int>(castle)] != File::NO_FILE;
        }

        constexpr bool has(Color color) const { return has(color, Side::KING_SIDE) || has(color, Side::QUEEN_SIDE); }

        constexpr File getRookFile(Color color, Side castle) const { return rooks[color][static_cast<int>(castle)]; }

        constexpr int hashIndex() const {
            return has(Color::WHITE, Side::KING_SIDE) + 2 * has(Color::WHITE, Side::QUEEN_SIDE) +
                   4 * has(Color::BLACK, Side::KING_SIDE) + 8 * has(Color::BLACK, Side::QUEEN_SIDE);
        }

        constexpr bool isEmpty() const { return !has(Color::WHITE) && !has(Color::BLACK); }

        template <typename T>
        static constexpr Side closestSide(T sq, T pred) {
            return sq > pred ? Side::KING_SIDE : Side::QUEEN_SIDE;
        }

       private:
        std::array<std::array<File, 2>, 2> rooks;
    };

   private:
    struct State {
        U64 hash;
        CastlingRights castling;
        Square enpassant;
        std::uint8_t half_moves;
        Piece captured_piece;

        State(const U64 &hash, const CastlingRights &castling, const Square &enpassant, const std::uint8_t &half_moves,
              const Piece &captured_piece)
            : hash(hash),
              castling(castling),
              enpassant(enpassant),
              half_moves(half_moves),
              captured_piece(captured_piece) {}
    };

    enum class PrivateCtor { CREATE };

    // private constructor to avoid initialization
    Board(PrivateCtor) {}

   public:
    explicit Board(std::string_view fen = constants::STARTPOS, bool chess960 = false) {
        prev_states_.reserve(256);
        chess960_ = chess960;
        assert(setFenInternal<true>(constants::STARTPOS));
        setFenInternal<true>(fen);
    }

    static Board fromFen(std::string_view fen) { return Board(fen); }
    static Board fromEpd(std::string_view epd) {
        Board board;
        board.setEpd(epd);
        return board;
    }

    /**
     * @brief Returns true if the given FEN was successfully parsed and set.
     * @param fen
     * @return
     */
    virtual bool setFen(std::string_view fen) { return setFenInternal(fen); }

    /**
     * @brief Returns true if the given EPD was successfully parsed and set.
     * @param epd
     * @return
     */
    bool setEpd(const std::string_view epd) {
        auto parts = utils::splitString(epd, ' ');

        if (parts.size() < 4) return false;

        int hm = 0;
        int fm = 1;

        if (auto it = std::find(parts.begin(), parts.end(), "hmvc"); it != parts.end()) {
            if (std::distance(it, parts.end()) > 1) {
                auto num    = *(it + 1);
                auto parsed = detail::parseStringViewToInt(num);
                if (parsed) hm = *parsed;
            } else {
                return false;
            }
        }

        if (auto it = std::find(parts.begin(), parts.end(), "fmvn"); it != parts.end()) {
            if (std::distance(it, parts.end()) > 1) {
                auto num    = *(it + 1);
                auto parsed = detail::parseStringViewToInt(num);
                if (parsed && *parsed > 0)
                    fm = *parsed;
                else
                    return false;
            } else {
                return false;
            }
        }

        auto fen = std::string(parts[0]) + " " + std::string(parts[1]) + " " + std::string(parts[2]) + " " +
                   std::string(parts[3]) + " " + std::to_string(hm) + " " + std::to_string(fm);

        return setFen(fen);
    }

    /**
     * @brief  Get the current FEN string.
     * @param move_counters
     * @return
     */
    [[nodiscard]] std::string getFen(bool move_counters = true) const {
        std::string ss;
        ss.reserve(100);

        // Loop through the ranks of the board in reverse order
        for (int rank = 7; rank >= 0; rank--) {
            std::uint32_t free_space = 0;

            // Loop through the files of the board
            for (int file = 0; file < 8; file++) {
                // Calculate the square index
                const int sq = rank * 8 + file;

                // If there is a piece at the current square
                if (Piece piece = at(Square(sq)); piece != Piece::NONE) {
                    // If there were any empty squares before this piece,
                    // append the number of empty squares to the FEN string
                    if (free_space) {
                        ss += std::to_string(free_space);
                        free_space = 0;
                    }

                    // Append the character representing the piece to the FEN string
                    ss += static_cast<std::string>(piece);
                } else {
                    // If there is no piece at the current square, increment the
                    // counter for the number of empty squares
                    free_space++;
                }
            }

            // If there are any empty squares at the end of the rank,
            // append the number of empty squares to the FEN string
            if (free_space != 0) {
                ss += std::to_string(free_space);
            }

            // Append a "/" character to the FEN string, unless this is the last rank
            ss += (rank > 0 ? "/" : "");
        }

        // Append " w " or " b " to the FEN string, depending on which player's turn it is
        ss += ' ';
        ss += (stm_ == Color::WHITE ? 'w' : 'b');

        // Append the appropriate characters to the FEN string to indicate
        // whether castling is allowed for each player
        if (cr_.isEmpty())
            ss += " -";
        else {
            ss += ' ';
            ss += getCastleString();
        }

        // Append information about the en passant square (if any)
        // and the half-move clock and full move number to the FEN string
        if (ep_sq_ == Square::NO_SQ)
            ss += " -";
        else {
            ss += ' ';
            ss += static_cast<std::string>(ep_sq_);
        }

        if (move_counters) {
            ss += ' ';
            ss += std::to_string(halfMoveClock());
            ss += ' ';
            ss += std::to_string(fullMoveNumber());
        }

        // Return the resulting FEN string
        return ss;
    }

    [[nodiscard]] std::string getEpd() const {
        std::string ss;
        ss.reserve(100);

        ss += getFen(false);
        ss += " hmvc ";
        ss += std::to_string(halfMoveClock()) + ";";
        ss += " fmvn ";
        ss += std::to_string(fullMoveNumber()) + ";";

        return ss;
    }

    /**
     * @brief Make a move on the board. The move must be legal otherwise the
     * behavior is undefined. EXACT can be set to true to only record
     * the enpassant square if the enemy can legally capture the pawn on their
     * next move.
     * @tparam EXACT
     * @param move
     */
    template <bool EXACT = false>
    void makeMove(const Move move) {
        const auto capture  = at(move.to()) != Piece::NONE && move.typeOf() != Move::CASTLING;
        const auto captured = at(move.to());
        const auto pt       = at<PieceType>(move.from());

        // Validate side to move
        assert((at(move.from()) < Piece::BLACKPAWN) == (stm_ == Color::WHITE));

        prev_states_.emplace_back(key_, cr_, ep_sq_, hfm_, captured);

        hfm_++;
        plies_++;

        if (ep_sq_ != Square::NO_SQ) key_ ^= Zobrist::enpassant(ep_sq_.file());
        ep_sq_ = Square::NO_SQ;

        if (capture) {
            removePiece(captured, move.to());

            hfm_ = 0;
            key_ ^= Zobrist::piece(captured, move.to());

            // remove castling rights if rook is captured
            if (captured.type() == PieceType::ROOK && Rank::back_rank(move.to().rank(), ~stm_)) {
                const auto king_sq = kingSq(~stm_);
                const auto file    = CastlingRights::closestSide(move.to(), king_sq);

                if (cr_.getRookFile(~stm_, file) == move.to().file()) {
                    key_ ^= Zobrist::castlingIndex(cr_.clear(~stm_, file));
                }
            }
        }

        // remove castling rights if king moves
        if (pt == PieceType::KING && cr_.has(stm_)) {
            key_ ^= Zobrist::castling(cr_.hashIndex());
            cr_.clear(stm_);
            key_ ^= Zobrist::castling(cr_.hashIndex());
        } else if (pt == PieceType::ROOK && Square::back_rank(move.from(), stm_)) {
            const auto king_sq = kingSq(stm_);
            const auto file    = CastlingRights::closestSide(move.from(), king_sq);

            // remove castling rights if rook moves from back rank
            if (cr_.getRookFile(stm_, file) == move.from().file()) {
                key_ ^= Zobrist::castlingIndex(cr_.clear(stm_, file));
            }
        } else if (pt == PieceType::PAWN) {
            hfm_ = 0;

            // double push
            if (Square::value_distance(move.to(), move.from()) == 16) {
                // imaginary attacks from the ep square from the pawn which moved
                Bitboard ep_mask = attacks::pawn(stm_, move.to().ep_square());

                // add enpassant hash if enemy pawns are attacking the square
                if (static_cast<bool>(ep_mask & pieces(PieceType::PAWN, ~stm_))) {
                    int found = -1;

                    // check if the enemy can legally capture the pawn on the next move
                    if constexpr (EXACT) {
                        const auto piece = at(move.from());

                        found = 0;

                        removePieceInternal(piece, move.from());
                        placePieceInternal(piece, move.to());

                        stm_ = ~stm_;

                        bool valid;

                        if (stm_ == Color::WHITE) {
                            valid = movegen::isEpSquareValid<Color::WHITE>(*this, move.to().ep_square());
                        } else {
                            valid = movegen::isEpSquareValid<Color::BLACK>(*this, move.to().ep_square());
                        }

                        if (valid) found = 1;

                        // undo
                        stm_ = ~stm_;

                        removePieceInternal(piece, move.to());
                        placePieceInternal(piece, move.from());
                    }

                    if (found != 0) {
                        assert(at(move.to().ep_square()) == Piece::NONE);
                        ep_sq_ = move.to().ep_square();
                        key_ ^= Zobrist::enpassant(move.to().ep_square().file());
                    }
                }
            }
        }

        if (move.typeOf() == Move::CASTLING) {
            assert(at<PieceType>(move.from()) == PieceType::KING);
            assert(at<PieceType>(move.to()) == PieceType::ROOK);

            const bool king_side = move.to() > move.from();
            const auto rookTo    = Square::castling_rook_square(king_side, stm_);
            const auto kingTo    = Square::castling_king_square(king_side, stm_);

            const auto king = at(move.from());
            const auto rook = at(move.to());

            removePiece(king, move.from());
            removePiece(rook, move.to());

            assert(king == Piece(PieceType::KING, stm_));
            assert(rook == Piece(PieceType::ROOK, stm_));

            placePiece(king, kingTo);
            placePiece(rook, rookTo);

            key_ ^= Zobrist::piece(king, move.from()) ^ Zobrist::piece(king, kingTo);
            key_ ^= Zobrist::piece(rook, move.to()) ^ Zobrist::piece(rook, rookTo);
        } else if (move.typeOf() == Move::PROMOTION) {
            const auto piece_pawn = Piece(PieceType::PAWN, stm_);
            const auto piece_prom = Piece(move.promotionType(), stm_);

            removePiece(piece_pawn, move.from());
            placePiece(piece_prom, move.to());

            key_ ^= Zobrist::piece(piece_pawn, move.from()) ^ Zobrist::piece(piece_prom, move.to());
        } else {
            assert(at(move.from()) != Piece::NONE);
            assert(at(move.to()) == Piece::NONE);

            const auto piece = at(move.from());

            removePiece(piece, move.from());
            placePiece(piece, move.to());

            key_ ^= Zobrist::piece(piece, move.from()) ^ Zobrist::piece(piece, move.to());
        }

        if (move.typeOf() == Move::ENPASSANT) {
            assert(at<PieceType>(move.to().ep_square()) == PieceType::PAWN);

            const auto piece = Piece(PieceType::PAWN, ~stm_);

            removePiece(piece, move.to().ep_square());

            key_ ^= Zobrist::piece(piece, move.to().ep_square());
        }

        key_ ^= Zobrist::sideToMove();
        stm_ = ~stm_;
    }

    void unmakeMove(const Move move) {
        const auto &prev = prev_states_.back();

        ep_sq_ = prev.enpassant;
        cr_    = prev.castling;
        hfm_   = prev.half_moves;
        stm_   = ~stm_;
        plies_--;

        if (move.typeOf() == Move::CASTLING) {
            const bool king_side    = move.to() > move.from();
            const auto rook_from_sq = Square(king_side ? File::FILE_F : File::FILE_D, move.from().rank());
            const auto king_to_sq   = Square(king_side ? File::FILE_G : File::FILE_C, move.from().rank());

            assert(at<PieceType>(rook_from_sq) == PieceType::ROOK);
            assert(at<PieceType>(king_to_sq) == PieceType::KING);

            const auto rook = at(rook_from_sq);
            const auto king = at(king_to_sq);

            removePiece(rook, rook_from_sq);
            removePiece(king, king_to_sq);

            assert(king == Piece(PieceType::KING, stm_));
            assert(rook == Piece(PieceType::ROOK, stm_));

            placePiece(king, move.from());
            placePiece(rook, move.to());

        } else if (move.typeOf() == Move::PROMOTION) {
            const auto pawn  = Piece(PieceType::PAWN, stm_);
            const auto piece = at(move.to());

            assert(piece.type() == move.promotionType());
            assert(piece.type() != PieceType::PAWN);
            assert(piece.type() != PieceType::KING);
            assert(piece.type() != PieceType::NONE);

            removePiece(piece, move.to());
            placePiece(pawn, move.from());

            if (prev.captured_piece != Piece::NONE) {
                assert(at(move.to()) == Piece::NONE);
                placePiece(prev.captured_piece, move.to());
            }

        } else {
            assert(at(move.to()) != Piece::NONE);
            assert(at(move.from()) == Piece::NONE);

            const auto piece = at(move.to());

            removePiece(piece, move.to());
            placePiece(piece, move.from());

            if (move.typeOf() == Move::ENPASSANT) {
                const auto pawn   = Piece(PieceType::PAWN, ~stm_);
                const auto pawnTo = static_cast<Square>(ep_sq_ ^ 8);

                assert(at(pawnTo) == Piece::NONE);

                placePiece(pawn, pawnTo);
            } else if (prev.captured_piece != Piece::NONE) {
                assert(at(move.to()) == Piece::NONE);

                placePiece(prev.captured_piece, move.to());
            }
        }

        key_ = prev.hash;
        prev_states_.pop_back();
    }

    /**
     * @brief Make a null move. (Switches the side to move)
     */
    void makeNullMove() {
        prev_states_.emplace_back(key_, cr_, ep_sq_, hfm_, Piece::NONE);

        key_ ^= Zobrist::sideToMove();
        if (ep_sq_ != Square::NO_SQ) key_ ^= Zobrist::enpassant(ep_sq_.file());
        ep_sq_ = Square::NO_SQ;

        stm_ = ~stm_;

        plies_++;
    }

    /**
     * @brief Unmake a null move. (Switches the side to move)
     */
    void unmakeNullMove() {
        const auto &prev = prev_states_.back();

        ep_sq_ = prev.enpassant;
        cr_    = prev.castling;
        hfm_   = prev.half_moves;
        key_   = prev.hash;

        plies_--;

        stm_ = ~stm_;

        prev_states_.pop_back();
    }

    /**
     * @brief Get the occupancy bitboard for the color.
     * @param color
     * @return
     */
    [[nodiscard]] Bitboard us(Color color) const noexcept { return occ_bb_[color]; }

    /**
     * @brief Get the occupancy bitboard for the opposite color.
     * @param color
     * @return
     */
    [[nodiscard]] Bitboard them(Color color) const noexcept { return us(~color); }

    /**
     * @brief Get the occupancy bitboard for both colors.
     * Faster than calling all() or us(Color::WHITE) | us(Color::BLACK).
     * @return
     */
    [[nodiscard]] Bitboard occ() const noexcept { return occ_bb_[0] | occ_bb_[1]; }

    /**
     * @brief Get the occupancy bitboard for all pieces, should be only used internally.
     * @return
     */
    [[nodiscard]] Bitboard all() const noexcept { return us(Color::WHITE) | us(Color::BLACK); }

    /**
     * @brief Returns the square of the king for a certain color
     * @param color
     * @return
     */
    [[nodiscard]] Square kingSq(Color color) const noexcept {
        assert(pieces(PieceType::KING, color) != 0ull);
        return pieces(PieceType::KING, color).lsb();
    }

    /**
     * @brief Returns all pieces of a certain type and color
     * @param type
     * @param color
     * @return
     */
    [[nodiscard]] Bitboard pieces(PieceType type, Color color) const noexcept {
        return pieces_bb_[type] & occ_bb_[color];
    }

    /**
     * @brief Returns all pieces of a certain type
     * @param type
     * @return
     */
    [[nodiscard]] Bitboard pieces(PieceType type) const noexcept { return pieces_bb_[type]; }

    template <typename... Pieces, typename = std::enable_if_t<(std::is_convertible_v<Pieces, PieceType> && ...)>>
    [[nodiscard]] Bitboard pieces(Pieces... pieces) const noexcept {
        return (pieces_bb_[static_cast<PieceType>(pieces)] | ...);
    }

    /**
     * @brief Returns either the piece or the piece type on a square
     * @tparam T
     * @param sq
     * @return
     */
    template <typename T = Piece>
    [[nodiscard]] T at(Square sq) const noexcept {
        assert(sq.is_valid());

        if constexpr (std::is_same_v<T, PieceType>) {
            return board_[sq.index()].type();
        } else {
            return board_[sq.index()];
        }
    }

    /**
     * @brief Checks if a move is a capture, enpassant moves are also considered captures.
     * @param move
     * @return
     */
    bool isCapture(const Move move) const noexcept {
        return (at(move.to()) != Piece::NONE && move.typeOf() != Move::CASTLING) || move.typeOf() == Move::ENPASSANT;
    }

    /**
     * @brief Get the current zobrist hash key of the board
     * @return
     */
    [[nodiscard]] U64 hash() const noexcept { return key_; }
    [[nodiscard]] Color sideToMove() const noexcept { return stm_; }
    [[nodiscard]] Square enpassantSq() const noexcept { return ep_sq_; }
    [[nodiscard]] CastlingRights castlingRights() const noexcept { return cr_; }
    [[nodiscard]] std::uint32_t halfMoveClock() const noexcept { return hfm_; }
    [[nodiscard]] std::uint32_t fullMoveNumber() const noexcept { return 1 + plies_ / 2; }

    void set960(bool is960) {
        chess960_ = is960;
        if (!original_fen_.empty()) setFen(original_fen_);
    }

    /**
     * @brief Checks if the current position is a chess960, aka. FRC/DFRC position.
     * @return
     */
    [[nodiscard]] bool chess960() const noexcept { return chess960_; }

    /**
     * @brief Get the castling rights as a string
     * @return
     */
    [[nodiscard]] std::string getCastleString() const {
        static const auto get_file = [](const CastlingRights &cr, Color c, CastlingRights::Side side) {
            auto file = static_cast<std::string>(cr.getRookFile(c, side));
            return c == Color::WHITE ? std::toupper(file[0]) : file[0];
        };

        if (chess960_) {
            std::string ss;

            for (auto color : {Color::WHITE, Color::BLACK})
                for (auto side : {CastlingRights::Side::KING_SIDE, CastlingRights::Side::QUEEN_SIDE})
                    if (cr_.has(color, side)) ss += get_file(cr_, color, side);

            return ss;
        }

        std::string ss;

        if (cr_.has(Color::WHITE, CastlingRights::Side::KING_SIDE)) ss += 'K';
        if (cr_.has(Color::WHITE, CastlingRights::Side::QUEEN_SIDE)) ss += 'Q';
        if (cr_.has(Color::BLACK, CastlingRights::Side::KING_SIDE)) ss += 'k';
        if (cr_.has(Color::BLACK, CastlingRights::Side::QUEEN_SIDE)) ss += 'q';

        return ss;
    }

    /**
     * @brief Checks if the current position is a repetition, set this to 1 if
     * you are writing a chess engine.
     * @param count
     * @return
     */
    [[nodiscard]] bool isRepetition(int count = 2) const noexcept {
        std::uint8_t c = 0;

        // We start the loop from the back and go forward in moves, at most to the
        // last move which reset the half-move counter because repetitions cant
        // be across half-moves.
        const auto size = static_cast<int>(prev_states_.size());

        for (int i = size - 2; i >= 0 && i >= size - hfm_ - 1; i -= 2) {
            if (prev_states_[i].hash == key_) c++;
            if (c == count) return true;
        }

        return false;
    }

    /**
     * @brief Checks if the current position is a draw by 50 move rule.
     * Keep in mind that by the rules of chess, if the position has 50 half
     * moves it's not necessarily a draw, since checkmate has higher priority,
     * call getHalfMoveDrawType,
     * to determine whether the position is a draw or checkmate.
     * @return
     */
    [[nodiscard]] bool isHalfMoveDraw() const noexcept { return hfm_ >= 100; }

    /**
     * @brief Only call this function if isHalfMoveDraw() returns true.
     * @return
     */
    [[nodiscard]] std::pair<GameResultReason, GameResult> getHalfMoveDrawType() const noexcept {
        Movelist movelist;
        movegen::legalmoves(movelist, *this);

        if (movelist.empty() && inCheck()) {
            return {GameResultReason::CHECKMATE, GameResult::LOSE};
        }

        return {GameResultReason::FIFTY_MOVE_RULE, GameResult::DRAW};
    }

    /**
     * @brief Basic check if the current position is a draw by insufficient material.
     * @return
     */
    [[nodiscard]] bool isInsufficientMaterial() const noexcept {
        const auto count = occ().count();

        // only kings, draw
        if (count == 2) return true;

        // only bishop + knight, cant mate
        if (count == 3) {
            if (pieces(PieceType::BISHOP, Color::WHITE) || pieces(PieceType::BISHOP, Color::BLACK)) return true;
            if (pieces(PieceType::KNIGHT, Color::WHITE) || pieces(PieceType::KNIGHT, Color::BLACK)) return true;
        }

        // same colored bishops, cant mate
        if (count == 4) {
            if (pieces(PieceType::BISHOP, Color::WHITE) && pieces(PieceType::BISHOP, Color::BLACK) &&
                Square::same_color(pieces(PieceType::BISHOP, Color::WHITE).lsb(),
                                   pieces(PieceType::BISHOP, Color::BLACK).lsb()))
                return true;

            // one side with two bishops which have the same color
            auto white_bishops = pieces(PieceType::BISHOP, Color::WHITE);
            auto black_bishops = pieces(PieceType::BISHOP, Color::BLACK);

            if (white_bishops.count() == 2) {
                if (Square::same_color(white_bishops.lsb(), white_bishops.msb())) return true;
            } else if (black_bishops.count() == 2) {
                if (Square::same_color(black_bishops.lsb(), black_bishops.msb())) return true;
            }
        }

        return false;
    }

    /**
     * @brief Checks if the game is over. Returns GameResultReason::NONE if the game is not over.
     * This function calculates all legal moves for the current position to check if the game is over.
     * If you are writing a chess engine you should not use this function.
     * @return
     */
    [[nodiscard]] std::pair<GameResultReason, GameResult> isGameOver() const noexcept {
        if (isHalfMoveDraw()) return getHalfMoveDrawType();
        if (isInsufficientMaterial()) return {GameResultReason::INSUFFICIENT_MATERIAL, GameResult::DRAW};
        if (isRepetition()) return {GameResultReason::THREEFOLD_REPETITION, GameResult::DRAW};

        Movelist movelist;
        movegen::legalmoves(movelist, *this);

        if (movelist.empty()) {
            if (inCheck()) return {GameResultReason::CHECKMATE, GameResult::LOSE};
            return {GameResultReason::STALEMATE, GameResult::DRAW};
        }

        return {GameResultReason::NONE, GameResult::NONE};
    }

    /**
     * @brief Checks if a square is attacked by the given color.
     * @param square
     * @param color
     * @return
     */
    [[nodiscard]] bool isAttacked(Square square, Color color) const noexcept {
        // cheap checks first
        if (attacks::pawn(~color, square) & pieces(PieceType::PAWN, color)) return true;
        if (attacks::knight(square) & pieces(PieceType::KNIGHT, color)) return true;
        if (attacks::king(square) & pieces(PieceType::KING, color)) return true;
        if (attacks::bishop(square, occ()) & pieces(PieceType::BISHOP, PieceType::QUEEN) & us(color)) return true;
        if (attacks::rook(square, occ()) & pieces(PieceType::ROOK, PieceType::QUEEN) & us(color)) return true;

        return false;
    }

    /**
     * @brief Checks if the current side to move is in check
     * @return
     */
    [[nodiscard]] bool inCheck() const noexcept { return isAttacked(kingSq(stm_), ~stm_); }

    [[nodiscard]] CheckType givesCheck(const Move &m) const noexcept;

    /**
     * @brief Checks if the given color has at least 1 piece thats not pawn and not king
     * @param color
     * @return
     */
    [[nodiscard]] bool hasNonPawnMaterial(Color color) const noexcept {
        return bool(us(color) ^ (pieces(PieceType::PAWN, PieceType::KING) & us(color)));
    }

    /**
     * @brief Calculates the zobrist hash key of the board, expensive! Prefer using hash().
     * @return
     */
    [[nodiscard]] U64 zobrist() const {
        U64 hash_key = 0ULL;

        auto pieces = occ();

        while (pieces) {
            const Square sq = pieces.pop();
            hash_key ^= Zobrist::piece(at(sq), sq);
        }

        U64 ep_hash = 0ULL;
        if (ep_sq_ != Square::NO_SQ) ep_hash ^= Zobrist::enpassant(ep_sq_.file());

        U64 stm_hash = 0ULL;
        if (stm_ == Color::WHITE) stm_hash ^= Zobrist::sideToMove();

        U64 castling_hash = 0ULL;
        castling_hash ^= Zobrist::castling(cr_.hashIndex());

        return hash_key ^ ep_hash ^ stm_hash ^ castling_hash;
    }

    [[nodiscard]] Bitboard getCastlingPath(Color c, bool isKingSide) const noexcept {
        return castling_path[c][isKingSide];
    }

    friend std::ostream &operator<<(std::ostream &os, const Board &board);

    /**
     * @brief Compresses the board into a PackedBoard.
     */
    class Compact {
        friend class Board;
        Compact() = default;

       public:
        /**
         * @brief Compresses the board into a PackedBoard
         * @param board
         * @return
         */
        static PackedBoard encode(const Board &board) { return encodeState(board); }

        static PackedBoard encode(std::string_view fen, bool chess960 = false) { return encodeState(fen, chess960); }

        /**
         * @brief Creates a Board object from a PackedBoard
         * @param compressed
         * @param chess960 If the board is a chess960 position, set this to true
         * @return
         */
        static Board decode(const PackedBoard &compressed, bool chess960 = false) {
            Board board     = Board(PrivateCtor::CREATE);
            board.chess960_ = chess960;
            decode(board, compressed);
            return board;
        }

       private:
        /**
         * A compact board representation can be achieved in 24 bytes,
         * we use 8 bytes (64bit) to store the occupancy bitboard,
         * and 16 bytes (128bit) to store the pieces (plus some special information).
         *
         * Each of the 16 bytes can store 2 pieces, since chess only has 12 different pieces,
         * we can represent the pieces from 0 to 11 in 4 bits (a nibble) and use the other 4 bit for
         * the next piece.
         * Since we need to store information about enpassant, castling rights and the side to move,
         * we can use the remaining 4 bits to store this information.
         *
         * However we need to store the information and the piece information together.
         * This means in our case that
         * 12 -> enpassant + a pawn, we can deduce the color of the pawn from the rank of the square
         * 13 -> white rook with castling rights, we later use the file to deduce if it's a short or long castle
         * 14 -> black rook with castling rights, we later use the file to deduce if it's a short or long castle
         * 15 -> black king and black is side to move
         *
         * We will later deduce the square of the pieces from the occupancy bitboard.
         */
        static PackedBoard encodeState(const Board &board) {
            PackedBoard packed{};

            packed[0] = board.occ().getBits() >> 56;
            packed[1] = (board.occ().getBits() >> 48) & 0xFF;
            packed[2] = (board.occ().getBits() >> 40) & 0xFF;
            packed[3] = (board.occ().getBits() >> 32) & 0xFF;
            packed[4] = (board.occ().getBits() >> 24) & 0xFF;
            packed[5] = (board.occ().getBits() >> 16) & 0xFF;
            packed[6] = (board.occ().getBits() >> 8) & 0xFF;
            packed[7] = board.occ().getBits() & 0xFF;

            auto offset = 8 * 2;
            auto occ    = board.occ();

            while (occ) {
                // we now fill the packed array, since our convertedpiece only actually needs 4 bits,
                // we can store 2 pieces in one byte.
                const auto sq      = Square(occ.pop());
                const auto shift   = (offset % 2 == 0 ? 4 : 0);
                const auto meaning = convertMeaning(board.cr_, board.sideToMove(), board.ep_sq_, sq, board.at(sq));
                const auto nibble  = meaning << shift;

                packed[offset / 2] |= nibble;
                offset++;
            }

            return packed;
        }

        static PackedBoard encodeState(std::string_view fen, bool chess960 = false) {
            // fallback to slower method
            if (chess960) {
                return encodeState(Board(fen, true));
            }

            PackedBoard packed{};

            while (fen[0] == ' ') fen.remove_prefix(1);

            const auto params     = split_string_view<4>(fen);
            const auto position   = params[0].has_value() ? *params[0] : "";
            const auto move_right = params[1].has_value() ? *params[1] : "w";
            const auto castling   = params[2].has_value() ? *params[2] : "-";
            const auto en_passant = params[3].has_value() ? *params[3] : "-";

            const auto ep  = en_passant == "-" ? Square::NO_SQ : Square(en_passant);
            const auto stm = (move_right == "w") ? Color::WHITE : Color::BLACK;

            CastlingRights cr;

            for (char i : castling) {
                if (i == '-') break;

                const auto king_side  = CastlingRights::Side::KING_SIDE;
                const auto queen_side = CastlingRights::Side::QUEEN_SIDE;

                if (i == 'K') cr.setCastlingRight(Color::WHITE, king_side, File::FILE_H);
                if (i == 'Q') cr.setCastlingRight(Color::WHITE, queen_side, File::FILE_A);
                if (i == 'k') cr.setCastlingRight(Color::BLACK, king_side, File::FILE_H);
                if (i == 'q') cr.setCastlingRight(Color::BLACK, queen_side, File::FILE_A);

                assert(i == 'K' || i == 'Q' || i == 'k' || i == 'q');

                continue;
            }

            const auto parts = split_string_view<8>(position, '/');

            int offset   = 8 * 2;
            int square   = 0;
            Bitboard occ = 0ull;

            for (auto i = parts.rbegin(); i != parts.rend(); i++) {
                auto part = *i;

                for (char curr : *part) {
                    if (isdigit(curr)) {
                        square += (curr - '0');
                    } else if (curr == '/') {
                        square++;
                    } else {
                        const auto p     = Piece(std::string_view(&curr, 1));
                        const auto shift = (offset % 2 == 0 ? 4 : 0);

                        packed[offset / 2] |= convertMeaning(cr, stm, ep, Square(square), p) << shift;
                        offset++;

                        occ.set(square);
                        ++square;
                    }
                }
            }

            packed[0] = occ.getBits() >> 56;
            packed[1] = (occ.getBits() >> 48) & 0xFF;
            packed[2] = (occ.getBits() >> 40) & 0xFF;
            packed[3] = (occ.getBits() >> 32) & 0xFF;
            packed[4] = (occ.getBits() >> 24) & 0xFF;
            packed[5] = (occ.getBits() >> 16) & 0xFF;
            packed[6] = (occ.getBits() >> 8) & 0xFF;
            packed[7] = occ.getBits() & 0xFF;

            return packed;
        }

        static void decode(Board &board, const PackedBoard &compressed) {
            Bitboard occupied = 0ull;

            for (int i = 0; i < 8; i++) {
                occupied |= Bitboard(compressed[i]) << (56 - i * 8);
            }

            int offset           = 16;
            int white_castle_idx = 0, black_castle_idx = 0;
            File white_castle[2] = {File::NO_FILE, File::NO_FILE};
            File black_castle[2] = {File::NO_FILE, File::NO_FILE};

            // clear board state

            board.hfm_   = 0;
            board.plies_ = 0;

            board.stm_ = Color::WHITE;

            board.cr_.clear();
            board.prev_states_.clear();
            board.original_fen_.clear();

            board.occ_bb_.fill(0ULL);
            board.pieces_bb_.fill(0ULL);
            board.board_.fill(Piece::NONE);

            // place pieces back on the board
            while (occupied) {
                const auto sq     = Square(occupied.pop());
                const auto nibble = compressed[offset / 2] >> (offset % 2 == 0 ? 4 : 0) & 0b1111;
                const auto piece  = convertPiece(nibble);

                if (piece != Piece::NONE) {
                    board.placePiece(piece, sq);

                    offset++;
                    continue;
                }

                // Piece has a special meaning, interpret it from the raw integer
                // pawn with ep square behind it
                if (nibble == 12) {
                    board.ep_sq_ = sq.ep_square();
                    // depending on the rank this is a white or black pawn
                    auto color = sq.rank() == Rank::RANK_4 ? Color::WHITE : Color::BLACK;
                    board.placePiece(Piece(PieceType::PAWN, color), sq);
                }
                // castling rights for white
                else if (nibble == 13) {
                    assert(white_castle_idx < 2);
                    white_castle[white_castle_idx++] = sq.file();
                    board.placePiece(Piece(PieceType::ROOK, Color::WHITE), sq);
                }
                // castling rights for black
                else if (nibble == 14) {
                    assert(black_castle_idx < 2);
                    black_castle[black_castle_idx++] = sq.file();
                    board.placePiece(Piece(PieceType::ROOK, Color::BLACK), sq);
                }
                // black to move
                else if (nibble == 15) {
                    board.stm_ = Color::BLACK;
                    board.placePiece(Piece(PieceType::KING, Color::BLACK), sq);
                }

                offset++;
            }

            // reapply castling
            for (int i = 0; i < 2; i++) {
                if (white_castle[i] != File::NO_FILE) {
                    const auto king_sq = board.kingSq(Color::WHITE);
                    const auto file    = white_castle[i];
                    const auto side    = CastlingRights::closestSide(file, king_sq.file());

                    board.cr_.setCastlingRight(Color::WHITE, side, file);
                }

                if (black_castle[i] != File::NO_FILE) {
                    const auto king_sq = board.kingSq(Color::BLACK);
                    const auto file    = black_castle[i];
                    const auto side    = CastlingRights::closestSide(file, king_sq.file());

                    board.cr_.setCastlingRight(Color::BLACK, side, file);
                }
            }

            if (board.stm_ == Color::BLACK) {
                board.plies_++;
            }

            board.key_ = board.zobrist();
        }

        // 1:1 mapping of Piece::internal() to the compressed piece
        static std::uint8_t convertPiece(Piece piece) { return static_cast<int>(piece.internal()); }

        // for pieces with a special meaning return Piece::NONE since this is otherwise not used
        static Piece convertPiece(std::uint8_t piece) {
            if (piece >= 12) return Piece::NONE;
            return Piece(Piece::underlying(piece));
        }

        // 12 => theres an ep square behind the pawn, rank will be deduced from the rank
        // 13 => any white rook with castling rights, side will be deduced from the file
        // 14 => any black rook with castling rights, side will be deduced from the file
        // 15 => black king and black is side to move
        static std::uint8_t convertMeaning(const CastlingRights &cr, Color stm, Square ep, Square sq, Piece piece) {
            if (piece.type() == PieceType::PAWN && ep != Square::NO_SQ) {
                if (Square(static_cast<int>(sq.index()) ^ 8) == ep) return 12;
            }

            if (piece.type() == PieceType::ROOK) {
                if (piece.color() == Color::WHITE && Square::back_rank(sq, Color::WHITE) &&
                    (cr.getRookFile(Color::WHITE, CastlingRights::Side::KING_SIDE) == sq.file() ||
                     cr.getRookFile(Color::WHITE, CastlingRights::Side::QUEEN_SIDE) == sq.file()))
                    return 13;
                if (piece.color() == Color::BLACK && Square::back_rank(sq, Color::BLACK) &&
                    (cr.getRookFile(Color::BLACK, CastlingRights::Side::KING_SIDE) == sq.file() ||
                     cr.getRookFile(Color::BLACK, CastlingRights::Side::QUEEN_SIDE) == sq.file()))
                    return 14;
            }

            if (piece.type() == PieceType::KING && piece.color() == Color::BLACK && stm == Color::BLACK) {
                return 15;
            }

            return convertPiece(piece);
        }
    };

   protected:
    virtual void placePiece(Piece piece, Square sq) { placePieceInternal(piece, sq); }

    virtual void removePiece(Piece piece, Square sq) { removePieceInternal(piece, sq); }

    std::vector<State> prev_states_;

    std::array<Bitboard, 6> pieces_bb_ = {};
    std::array<Bitboard, 2> occ_bb_    = {};
    std::array<Piece, 64> board_       = {};

    U64 key_             = 0ULL;
    CastlingRights cr_   = {};
    std::uint16_t plies_ = 0;
    Color stm_           = Color::WHITE;
    Square ep_sq_        = Square::NO_SQ;
    std::uint8_t hfm_    = 0;

    bool chess960_ = false;

    std::array<std::array<Bitboard, 2>, 2> castling_path = {};

   private:
    void removePieceInternal(Piece piece, Square sq) {
        assert(board_[sq.index()] == piece && piece != Piece::NONE);

        auto type  = piece.type();
        auto color = piece.color();
        auto index = sq.index();

        assert(type != PieceType::NONE);
        assert(color != Color::NONE);
        assert(index >= 0 && index < 64);

        pieces_bb_[type].clear(index);
        occ_bb_[color].clear(index);
        board_[index] = Piece::NONE;
    }

    void placePieceInternal(Piece piece, Square sq) {
        assert(board_[sq.index()] == Piece::NONE);

        auto type  = piece.type();
        auto color = piece.color();
        auto index = sq.index();

        assert(type != PieceType::NONE);
        assert(color != Color::NONE);
        assert(index >= 0 && index < 64);

        pieces_bb_[type].set(index);
        occ_bb_[color].set(index);
        board_[index] = piece;
    }

    template <bool ctor = false>
    bool setFenInternal(std::string_view fen) {
        original_fen_ = fen;

        reset();

        while (!fen.empty() && fen[0] == ' ') fen.remove_prefix(1);

        if (fen.empty()) return false;

        const auto params     = split_string_view<6>(fen);
        const auto position   = params[0].has_value() ? *params[0] : "";
        const auto move_right = params[1].has_value() ? *params[1] : "w";
        const auto castling   = params[2].has_value() ? *params[2] : "-";
        const auto en_passant = params[3].has_value() ? *params[3] : "-";
        const auto half_move  = params[4].has_value() ? *params[4] : "0";
        const auto full_move  = params[5].has_value() ? *params[5] : "1";

        if (position.empty()) return false;

        if (move_right != "w" && move_right != "b") return false;

        const auto half_move_opt = detail::parseStringViewToInt(half_move).value_or(0);
        hfm_                     = half_move_opt;

        const auto full_move_opt = detail::parseStringViewToInt(full_move).value_or(1);
        plies_                   = full_move_opt;

        plies_ = plies_ * 2 - 2;

        if (en_passant != "-") {
            if (!Square::is_valid_string_sq(en_passant)) {
                return false;
            }

            ep_sq_ = Square(en_passant);
            if (ep_sq_ == Square::NO_SQ) return false;
        }

        stm_ = (move_right == "w") ? Color::WHITE : Color::BLACK;

        if (stm_ == Color::BLACK) {
            plies_++;
        } else {
            key_ ^= Zobrist::sideToMove();
        }

        auto square = 56;
        for (char curr : position) {
            if (isdigit(curr)) {
                square += (curr - '0');
            } else if (curr == '/') {
                square -= 16;
            } else {
                auto p = Piece(std::string_view(&curr, 1));
                if (p == Piece::NONE || !Square::is_valid_sq(square) || at(square) != Piece::NONE) return false;

                if constexpr (ctor) {
                    placePieceInternal(p, Square(square));
                } else {
                    placePiece(p, square);
                }

                key_ ^= Zobrist::piece(p, Square(square));
                ++square;
            }
        }

        static const auto find_rook = [](const Board &board, CastlingRights::Side side, Color color) -> File {
            const auto king_side = CastlingRights::Side::KING_SIDE;
            const auto king_sq   = board.kingSq(color);
            const auto sq_corner = Square(side == king_side ? Square::SQ_H1 : Square::SQ_A1).relative_square(color);

            const auto start = side == king_side ? king_sq + 1 : king_sq - 1;

            for (Square sq = start; (side == king_side ? sq <= sq_corner : sq >= sq_corner);
                 (side == king_side ? sq++ : sq--)) {
                if (board.at<PieceType>(sq) == PieceType::ROOK && board.at(sq).color() == color) {
                    return sq.file();
                }
            }

            return File(File::NO_FILE);
        };

        // Parse castling rights
        for (char i : castling) {
            if (i == '-') break;

            const auto king_side  = CastlingRights::Side::KING_SIDE;
            const auto queen_side = CastlingRights::Side::QUEEN_SIDE;

            if (!chess960_) {
                if (i == 'K')
                    cr_.setCastlingRight(Color::WHITE, king_side, File::FILE_H);
                else if (i == 'Q')
                    cr_.setCastlingRight(Color::WHITE, queen_side, File::FILE_A);
                else if (i == 'k')
                    cr_.setCastlingRight(Color::BLACK, king_side, File::FILE_H);
                else if (i == 'q')
                    cr_.setCastlingRight(Color::BLACK, queen_side, File::FILE_A);
                else
                    return false;

                continue;
            }

            // chess960 castling detection
            const auto color   = isupper(i) ? Color::WHITE : Color::BLACK;
            const auto king_sq = kingSq(color);

            if (i == 'K' || i == 'k') {
                auto file = find_rook(*this, king_side, color);
                if (file == File::NO_FILE) return false;
                cr_.setCastlingRight(color, king_side, file);
            } else if (i == 'Q' || i == 'q') {
                auto file = find_rook(*this, queen_side, color);
                if (file == File::NO_FILE) return false;
                cr_.setCastlingRight(color, queen_side, file);
            } else {
                const auto file = File(std::string_view(&i, 1));
                if (file == File::NO_FILE) return false;
                const auto side = CastlingRights::closestSide(file, king_sq.file());
                cr_.setCastlingRight(color, side, file);
            }
        }

        if (ep_sq_ != Square::NO_SQ && !((ep_sq_.rank() == Rank::RANK_3 && stm_ == Color::BLACK) ||
                                         (ep_sq_.rank() == Rank::RANK_6 && stm_ == Color::WHITE))) {
            ep_sq_ = Square::NO_SQ;
        }

        if (ep_sq_ != Square::NO_SQ) {
            bool valid;

            if (stm_ == Color::WHITE) {
                valid = movegen::isEpSquareValid<Color::WHITE>(*this, ep_sq_);
            } else {
                valid = movegen::isEpSquareValid<Color::BLACK>(*this, ep_sq_);
            }

            if (!valid)
                ep_sq_ = Square::NO_SQ;
            else
                key_ ^= Zobrist::enpassant(ep_sq_.file());
        }

        key_ ^= Zobrist::castling(cr_.hashIndex());

        assert(key_ == zobrist());

        // init castling_path
        for (Color c : {Color::WHITE, Color::BLACK}) {
            const auto king_from = kingSq(c);

            for (const auto side : {CastlingRights::Side::KING_SIDE, CastlingRights::Side::QUEEN_SIDE}) {
                if (!cr_.has(c, side)) continue;

                const auto rook_from = Square(cr_.getRookFile(c, side), king_from.rank());
                const auto king_to   = Square::castling_king_square(side == CastlingRights::Side::KING_SIDE, c);
                const auto rook_to   = Square::castling_rook_square(side == CastlingRights::Side::KING_SIDE, c);

                castling_path[c][side == CastlingRights::Side::KING_SIDE] =
                    (movegen::between(rook_from, rook_to) | movegen::between(king_from, king_to)) &
                    ~(Bitboard::fromSquare(king_from) | Bitboard::fromSquare(rook_from));
            }
        }

        return true;
    }

    template <int N>
    std::array<std::optional<std::string_view>, N> static split_string_view(std::string_view fen,
                                                                            char delimiter = ' ') {
        std::array<std::optional<std::string_view>, N> arr = {};

        std::size_t start = 0;
        std::size_t end   = 0;

        for (std::size_t i = 0; i < N; i++) {
            end = fen.find(delimiter, start);
            if (end == std::string::npos) {
                arr[i] = fen.substr(start);
                break;
            }
            arr[i] = fen.substr(start, end - start);
            start  = end + 1;
        }

        return arr;
    }

    void reset() {
        occ_bb_.fill(0ULL);
        pieces_bb_.fill(0ULL);
        board_.fill(Piece::NONE);

        stm_   = Color::WHITE;
        ep_sq_ = Square::NO_SQ;
        hfm_   = 0;
        plies_ = 1;
        key_   = 0ULL;
        cr_.clear();
        prev_states_.clear();
    }

    // store the original fen string
    // useful when setting up a frc position and the user called set960(true) afterwards
    std::string original_fen_;
};

inline std::ostream &operator<<(std::ostream &os, const Board &b) {
    for (int i = 63; i >= 0; i -= 8) {
        for (int j = 7; j >= 0; j--) {
            os << " " << static_cast<std::string>(b.board_[i - j]);
        }

        os << "\n";
    }

    os << "\n\n";
    os << "Side to move: " << static_cast<int>(b.stm_.internal()) << "\n";
    os << "Castling rights: " << b.getCastleString() << "\n";
    os << "Halfmoves: " << b.halfMoveClock() << "\n";
    os << "Fullmoves: " << b.fullMoveNumber() << "\n";
    os << "EP: " << b.ep_sq_.index() << "\n";
    os << "Hash: " << b.key_ << "\n";

    os << std::endl;

    return os;
}

inline CheckType Board::givesCheck(const Move &m) const noexcept {
    const static auto getSniper = [](const Board *board, Square ksq, Bitboard oc) {
        const auto us_occ = board->us(board->sideToMove());
        const auto bishop = attacks::bishop(ksq, oc) & board->pieces(PieceType::BISHOP, PieceType::QUEEN) & us_occ;
        const auto rook   = attacks::rook(ksq, oc) & board->pieces(PieceType::ROOK, PieceType::QUEEN) & us_occ;
        return (bishop | rook);
    };

    assert(at(m.from()).color() == stm_);

    const Square from   = m.from();
    const Square to     = m.to();
    const Square ksq    = kingSq(~stm_);
    const Bitboard toBB = Bitboard::fromSquare(to);
    const PieceType pt  = at(from).type();

    Bitboard fromKing = 0ull;

    if (pt == PieceType::PAWN) {
        fromKing = attacks::pawn(~stm_, ksq);
    } else if (pt == PieceType::KNIGHT) {
        fromKing = attacks::knight(ksq);
    } else if (pt == PieceType::BISHOP) {
        fromKing = attacks::bishop(ksq, occ());
    } else if (pt == PieceType::ROOK) {
        fromKing = attacks::rook(ksq, occ());
    } else if (pt == PieceType::QUEEN) {
        fromKing = attacks::queen(ksq, occ());
    }

    if (fromKing & toBB) return CheckType::DIRECT_CHECK;

    // Discovery check
    const Bitboard fromBB = Bitboard::fromSquare(from);
    const Bitboard oc     = occ() ^ fromBB;

    Bitboard sniper = getSniper(this, ksq, oc);

    while (sniper) {
        Square sq = sniper.pop();
        return (!(movegen::between(ksq, sq) & toBB) || m.typeOf() == Move::CASTLING) ? CheckType::DISCOVERY_CHECK
                                                                                     : CheckType::NO_CHECK;
    }

    switch (m.typeOf()) {
        case Move::NORMAL:
            return CheckType::NO_CHECK;

        case Move::PROMOTION: {
            Bitboard attacks = 0ull;

            switch (m.promotionType()) {
                case static_cast<int>(PieceType::KNIGHT):
                    attacks = attacks::knight(to);
                    break;
                case static_cast<int>(PieceType::BISHOP):
                    attacks = attacks::bishop(to, oc);
                    break;
                case static_cast<int>(PieceType::ROOK):
                    attacks = attacks::rook(to, oc);
                    break;
                case static_cast<int>(PieceType::QUEEN):
                    attacks = attacks::queen(to, oc);
            }

            return (attacks & pieces(PieceType::KING, ~stm_)) ? CheckType::DIRECT_CHECK : CheckType::NO_CHECK;
        }

        case Move::ENPASSANT: {
            Square capSq(to.file(), from.rank());
            return (getSniper(this, ksq, (oc ^ Bitboard::fromSquare(capSq)) | toBB)) ? CheckType::DISCOVERY_CHECK
                                                                                     : CheckType::NO_CHECK;
        }

        case Move::CASTLING: {
            Square rookTo = Square::castling_rook_square(to > from, stm_);
            return (attacks::rook(ksq, occ()) & Bitboard::fromSquare(rookTo)) ? CheckType::DISCOVERY_CHECK
                                                                              : CheckType::NO_CHECK;
        }
    }

    assert(false);
    return CheckType::NO_CHECK;  // Prevent a compiler warning
}

}  // namespace  chess

namespace chess {

template <Direction direction>
[[nodiscard]] inline constexpr Bitboard attacks::shift(const Bitboard b) {
    switch (direction) {
        case Direction::NORTH:
            return b << 8;
        case Direction::SOUTH:
            return b >> 8;
        case Direction::NORTH_WEST:
            return (b & ~MASK_FILE[0]) << 7;
        case Direction::WEST:
            return (b & ~MASK_FILE[0]) >> 1;
        case Direction::SOUTH_WEST:
            return (b & ~MASK_FILE[0]) >> 9;
        case Direction::NORTH_EAST:
            return (b & ~MASK_FILE[7]) << 9;
        case Direction::EAST:
            return (b & ~MASK_FILE[7]) << 1;
        case Direction::SOUTH_EAST:
            return (b & ~MASK_FILE[7]) >> 7;
    }

    assert(false);

    return {};
}

template <Color::underlying c>
[[nodiscard]] inline Bitboard attacks::pawnLeftAttacks(const Bitboard pawns) {
    return c == Color::WHITE ? (pawns << 7) & ~MASK_FILE[static_cast<int>(File::FILE_H)]
                             : (pawns >> 7) & ~MASK_FILE[static_cast<int>(File::FILE_A)];
}

template <Color::underlying c>
[[nodiscard]] inline Bitboard attacks::pawnRightAttacks(const Bitboard pawns) {
    return c == Color::WHITE ? (pawns << 9) & ~MASK_FILE[static_cast<int>(File::FILE_A)]
                             : (pawns >> 9) & ~MASK_FILE[static_cast<int>(File::FILE_H)];
}

[[nodiscard]] inline Bitboard attacks::pawn(Color c, Square sq) noexcept { return PawnAttacks[c][sq.index()]; }

[[nodiscard]] inline Bitboard attacks::knight(Square sq) noexcept { return KnightAttacks[sq.index()]; }

[[nodiscard]] inline Bitboard attacks::bishop(Square sq, Bitboard occupied) noexcept {
    return BishopTable[sq.index()].attacks[BishopTable[sq.index()](occupied)];
}

[[nodiscard]] inline Bitboard attacks::rook(Square sq, Bitboard occupied) noexcept {
    return RookTable[sq.index()].attacks[RookTable[sq.index()](occupied)];
}

[[nodiscard]] inline Bitboard attacks::queen(Square sq, Bitboard occupied) noexcept {
    return bishop(sq, occupied) | rook(sq, occupied);
}

[[nodiscard]] inline Bitboard attacks::king(Square sq) noexcept { return KingAttacks[sq.index()]; }

[[nodiscard]] inline Bitboard attacks::attackers(const Board &board, Color color, Square square) noexcept {
    const auto queens   = board.pieces(PieceType::QUEEN, color);
    const auto occupied = board.occ();

    // using the fact that if we can attack PieceType from square, they can attack us back
    auto atks = (pawn(~color, square) & board.pieces(PieceType::PAWN, color));
    atks |= (knight(square) & board.pieces(PieceType::KNIGHT, color));
    atks |= (bishop(square, occupied) & (board.pieces(PieceType::BISHOP, color) | queens));
    atks |= (rook(square, occupied) & (board.pieces(PieceType::ROOK, color) | queens));
    atks |= (king(square) & board.pieces(PieceType::KING, color));

    return atks & occupied;
}

template <PieceType::underlying pt>
[[nodiscard]] inline Bitboard attacks::slider(Square sq, Bitboard occupied) noexcept {
    static_assert(pt == PieceType::BISHOP || pt == PieceType::ROOK || pt == PieceType::QUEEN,
                  "PieceType must be a slider!");

    if constexpr (pt == PieceType::BISHOP) return bishop(sq, occupied);
    if constexpr (pt == PieceType::ROOK) return rook(sq, occupied);
    if constexpr (pt == PieceType::QUEEN) return queen(sq, occupied);
}

template <bool ISROOK>
[[nodiscard]] inline Bitboard attacks::sliderAttacks(Square sq, Bitboard occupied) noexcept {
    static constexpr int dirs[2][4][2] = {{{1, 1}, {1, -1}, {-1, -1}, {-1, 1}}, {{1, 0}, {0, -1}, {-1, 0}, {0, 1}}};

    Bitboard attacks = 0ull;

    File pf = sq.file();
    Rank pr = sq.rank();

    for (int i = 0; i < 4; ++i) {
        int off_f = dirs[ISROOK][i][0];
        int off_r = dirs[ISROOK][i][1];

        File f;
        Rank r;
        for (f = pf + off_f, r = pr + off_r; Square::is_valid(r, f); f += off_f, r += off_r) {
            const auto index = Square(f, r).index();
            attacks.set(index);
            if (occupied.check(index)) break;
        }
    }

    return attacks;
}

inline void attacks::initSliders(Square sq, Magic table[], U64 magic,
                                 const std::function<Bitboard(Square, Bitboard)> &attacks) {
    // The edges of the board are not considered for the attacks
    // i.e. for the sq h7 edges will be a1-h1, a1-a8, a8-h8, ignoring the edge of the current square
    const Bitboard edges = ((Bitboard(Rank::RANK_1) | Bitboard(Rank::RANK_8)) & ~Bitboard(sq.rank())) |
                           ((Bitboard(File::FILE_A) | Bitboard(File::FILE_H)) & ~Bitboard(sq.file()));

    U64 occ = 0ULL;

    auto &table_sq = table[sq.index()];

#ifndef CHESS_USE_PEXT
    table_sq.magic = magic;
#endif
    table_sq.mask = (attacks(sq, occ) & ~edges).getBits();
#ifndef CHESS_USE_PEXT
    table_sq.shift = 64 - Bitboard(table_sq.mask).count();
#endif

    if (sq < 64 - 1) {
        table[sq.index() + 1].attacks = table_sq.attacks + (1ull << Bitboard(table_sq.mask).count());
    }

    do {
        table_sq.attacks[table_sq(occ)] = attacks(sq, occ);
        occ                             = (occ - table_sq.mask) & table_sq.mask;
    } while (occ);
}

inline void attacks::initAttacks() {
    BishopTable[0].attacks = BishopAttacks;
    RookTable[0].attacks   = RookAttacks;

    for (int i = 0; i < 64; i++) {
        initSliders(static_cast<Square>(i), BishopTable, BishopMagics[i], sliderAttacks<false>);
        initSliders(static_cast<Square>(i), RookTable, RookMagics[i], sliderAttacks<true>);
    }
}
}  // namespace chess



namespace chess {

inline auto movegen::init_squares_between() {
    std::array<std::array<Bitboard, 64>, 64> squares_between_bb{};

    auto att = [](PieceType pt, Square sq, Bitboard occ) {
        return (pt == PieceType::BISHOP) ? attacks::bishop(sq, occ) : attacks::rook(sq, occ);
    };

    for (int sq1 = 0; sq1 < 64; ++sq1) {
        for (PieceType pt : {PieceType::BISHOP, PieceType::ROOK}) {
            for (int sq2 = 0; sq2 < 64; ++sq2) {
                if (att(pt, sq1, 0).check(sq2)) {
                    squares_between_bb[sq1][sq2] =
                        att(pt, sq1, Bitboard::fromSquare(sq2)) & att(pt, sq2, Bitboard::fromSquare(sq1));
                }
                squares_between_bb[sq1][sq2].set(sq2);
            }
        }
    }

    return squares_between_bb;
}

template <Color::underlying c>
[[nodiscard]] inline std::pair<Bitboard, int> movegen::checkMask(const Board &board, Square sq) {
    const auto opp_knight = board.pieces(PieceType::KNIGHT, ~c);
    const auto opp_bishop = board.pieces(PieceType::BISHOP, ~c);
    const auto opp_rook   = board.pieces(PieceType::ROOK, ~c);
    const auto opp_queen  = board.pieces(PieceType::QUEEN, ~c);

    const auto opp_pawns = board.pieces(PieceType::PAWN, ~c);

    int checks = 0;

    // check for knight checks
    Bitboard knight_attacks = attacks::knight(sq) & opp_knight;
    checks += bool(knight_attacks);

    Bitboard mask = knight_attacks;

    // check for pawn checks
    Bitboard pawn_attacks = attacks::pawn(board.sideToMove(), sq) & opp_pawns;
    mask |= pawn_attacks;
    checks += bool(pawn_attacks);

    // check for bishop checks
    Bitboard bishop_attacks = attacks::bishop(sq, board.occ()) & (opp_bishop | opp_queen);

    if (bishop_attacks) {
        mask |= between(sq, bishop_attacks.lsb());
        checks++;
    }

    Bitboard rook_attacks = attacks::rook(sq, board.occ()) & (opp_rook | opp_queen);

    if (rook_attacks) {
        if (rook_attacks.count() > 1) {
            checks = 2;
            return {mask, checks};
        }

        mask |= between(sq, rook_attacks.lsb());
        checks++;
    }

    if (!mask) {
        return {constants::DEFAULT_CHECKMASK, checks};
    }

    return {mask, checks};
}

template <Color::underlying c, PieceType::underlying pt>
[[nodiscard]] inline Bitboard movegen::pinMask(const Board &board, Square sq, Bitboard occ_opp,
                                               Bitboard occ_us) noexcept {
    static_assert(pt == PieceType::BISHOP || pt == PieceType::ROOK, "Only bishop or rook allowed!");

    const auto opp_pt_queen = board.pieces(pt, PieceType::QUEEN) & board.us(~c);

    auto pt_attacks = attacks::slider<pt>(sq, occ_opp) & opp_pt_queen;

    Bitboard pin = 0ull;

    while (pt_attacks) {
        const auto possible_pin = between(sq, pt_attacks.pop());
        if ((possible_pin & occ_us).count() == 1) pin |= possible_pin;
    }

    return pin;
}

template <Color::underlying c>
[[nodiscard]] inline Bitboard movegen::seenSquares(const Board &board, Bitboard enemy_empty) {
    auto king_sq          = board.kingSq(~c);
    Bitboard map_king_atk = attacks::king(king_sq) & enemy_empty;

    if (map_king_atk == Bitboard(0ull) && !board.chess960()) return 0ull;

    auto occ     = board.occ() ^ Bitboard::fromSquare(king_sq);
    auto queens  = board.pieces(PieceType::QUEEN, c);
    auto pawns   = board.pieces(PieceType::PAWN, c);
    auto knights = board.pieces(PieceType::KNIGHT, c);
    auto bishops = board.pieces(PieceType::BISHOP, c) | queens;
    auto rooks   = board.pieces(PieceType::ROOK, c) | queens;

    Bitboard seen = attacks::pawnLeftAttacks<c>(pawns) | attacks::pawnRightAttacks<c>(pawns);

    while (knights) {
        seen |= attacks::knight(knights.pop());
    }

    while (bishops) {
        seen |= attacks::bishop(bishops.pop(), occ);
    }

    while (rooks) {
        seen |= attacks::rook(rooks.pop(), occ);
    }

    seen |= attacks::king(board.kingSq(c));

    return seen;
}

template <Color::underlying c, movegen::MoveGenType mt>
inline void movegen::generatePawnMoves(const Board &board, Movelist &moves, Bitboard pin_d, Bitboard pin_hv,
                                       Bitboard checkmask, Bitboard occ_opp) {
    // flipped for black

    constexpr auto UP         = make_direction(Direction::NORTH, c);
    constexpr auto DOWN       = make_direction(Direction::SOUTH, c);
    constexpr auto DOWN_LEFT  = make_direction(Direction::SOUTH_WEST, c);
    constexpr auto DOWN_RIGHT = make_direction(Direction::SOUTH_EAST, c);
    constexpr auto UP_LEFT    = make_direction(Direction::NORTH_WEST, c);
    constexpr auto UP_RIGHT   = make_direction(Direction::NORTH_EAST, c);

    constexpr auto RANK_B_PROMO     = Rank::rank(Rank::RANK_7, c).bb();
    constexpr auto RANK_PROMO       = Rank::rank(Rank::RANK_8, c).bb();
    constexpr auto DOUBLE_PUSH_RANK = Rank::rank(Rank::RANK_3, c).bb();

    const auto pawns = board.pieces(PieceType::PAWN, c);

    // These pawns can maybe take Left or Right
    const Bitboard pawns_lr          = pawns & ~pin_hv;
    const Bitboard unpinned_pawns_lr = pawns_lr & ~pin_d;
    const Bitboard pinned_pawns_lr   = pawns_lr & pin_d;

    auto l_pawns = attacks::shift<UP_LEFT>(unpinned_pawns_lr) | (attacks::shift<UP_LEFT>(pinned_pawns_lr) & pin_d);
    auto r_pawns = attacks::shift<UP_RIGHT>(unpinned_pawns_lr) | (attacks::shift<UP_RIGHT>(pinned_pawns_lr) & pin_d);

    // Prune moves that don't capture a piece and are not on the checkmask.
    l_pawns &= occ_opp & checkmask;
    r_pawns &= occ_opp & checkmask;

    // These pawns can walk Forward
    const auto pawns_hv = pawns & ~pin_d;

    const auto pawns_pinned_hv   = pawns_hv & pin_hv;
    const auto pawns_unpinned_hv = pawns_hv & ~pin_hv;

    // Prune moves that are blocked by a piece
    const auto single_push_unpinned = attacks::shift<UP>(pawns_unpinned_hv) & ~board.occ();
    const auto single_push_pinned   = attacks::shift<UP>(pawns_pinned_hv) & pin_hv & ~board.occ();

    // Prune moves that are not on the checkmask.
    Bitboard single_push = (single_push_unpinned | single_push_pinned) & checkmask;

    Bitboard double_push = ((attacks::shift<UP>(single_push_unpinned & DOUBLE_PUSH_RANK) & ~board.occ()) |
                            (attacks::shift<UP>(single_push_pinned & DOUBLE_PUSH_RANK) & ~board.occ())) &
                           checkmask;

    if (pawns & RANK_B_PROMO) {
        Bitboard promo_left  = l_pawns & RANK_PROMO;
        Bitboard promo_right = r_pawns & RANK_PROMO;
        Bitboard promo_push  = single_push & RANK_PROMO;

        // Skip capturing promotions if we are only generating quiet moves.
        // Generates at ALL and CAPTURE
        while (mt != MoveGenType::QUIET && promo_left) {
            const auto index = promo_left.pop();
            moves.add(Move::make<Move::PROMOTION>(index + DOWN_RIGHT, index, PieceType::QUEEN));
            moves.add(Move::make<Move::PROMOTION>(index + DOWN_RIGHT, index, PieceType::ROOK));
            moves.add(Move::make<Move::PROMOTION>(index + DOWN_RIGHT, index, PieceType::BISHOP));
            moves.add(Move::make<Move::PROMOTION>(index + DOWN_RIGHT, index, PieceType::KNIGHT));
        }

        // Skip capturing promotions if we are only generating quiet moves.
        // Generates at ALL and CAPTURE
        while (mt != MoveGenType::QUIET && promo_right) {
            const auto index = promo_right.pop();
            moves.add(Move::make<Move::PROMOTION>(index + DOWN_LEFT, index, PieceType::QUEEN));
            moves.add(Move::make<Move::PROMOTION>(index + DOWN_LEFT, index, PieceType::ROOK));
            moves.add(Move::make<Move::PROMOTION>(index + DOWN_LEFT, index, PieceType::BISHOP));
            moves.add(Move::make<Move::PROMOTION>(index + DOWN_LEFT, index, PieceType::KNIGHT));
        }

        // Skip quiet promotions if we are only generating captures.
        // Generates at ALL and QUIET
        while (mt != MoveGenType::CAPTURE && promo_push) {
            const auto index = promo_push.pop();
            moves.add(Move::make<Move::PROMOTION>(index + DOWN, index, PieceType::QUEEN));
            moves.add(Move::make<Move::PROMOTION>(index + DOWN, index, PieceType::ROOK));
            moves.add(Move::make<Move::PROMOTION>(index + DOWN, index, PieceType::BISHOP));
            moves.add(Move::make<Move::PROMOTION>(index + DOWN, index, PieceType::KNIGHT));
        }
    }

    single_push &= ~RANK_PROMO;
    l_pawns &= ~RANK_PROMO;
    r_pawns &= ~RANK_PROMO;

    while (mt != MoveGenType::QUIET && l_pawns) {
        const auto index = l_pawns.pop();
        moves.add(Move::make<Move::NORMAL>(index + DOWN_RIGHT, index));
    }

    while (mt != MoveGenType::QUIET && r_pawns) {
        const auto index = r_pawns.pop();
        moves.add(Move::make<Move::NORMAL>(index + DOWN_LEFT, index));
    }

    while (mt != MoveGenType::CAPTURE && single_push) {
        const auto index = single_push.pop();
        moves.add(Move::make<Move::NORMAL>(index + DOWN, index));
    }

    while (mt != MoveGenType::CAPTURE && double_push) {
        const auto index = double_push.pop();
        moves.add(Move::make<Move::NORMAL>(index + DOWN + DOWN, index));
    }

    if constexpr (mt == MoveGenType::QUIET) return;

    const Square ep = board.enpassantSq();

    if (ep != Square::NO_SQ) {
        auto m = generateEPMove(board, checkmask, pin_d, pawns_lr, ep, c);

        for (const auto &move : m) {
            if (move != Move::NO_MOVE) moves.add(move);
        }
    }
}

[[nodiscard]] inline std::array<Move, 2> movegen::generateEPMove(const Board &board, Bitboard checkmask, Bitboard pin_d,
                                                                 Bitboard pawns_lr, Square ep, Color c) {
    assert((ep.rank() == Rank::RANK_3 && board.sideToMove() == Color::BLACK) ||
           (ep.rank() == Rank::RANK_6 && board.sideToMove() == Color::WHITE));

    std::array<Move, 2> moves = {Move::NO_MOVE, Move::NO_MOVE};
    int i                     = 0;

    const auto DOWN     = make_direction(Direction::SOUTH, c);
    const auto epPawnSq = ep + DOWN;

    /*
     In case the en passant square and the enemy pawn
     that just moved are not on the checkmask
     en passant is not available.
    */
    if ((checkmask & (Bitboard::fromSquare(epPawnSq) | Bitboard::fromSquare(ep))) == 0ull) return moves;

    const Square kSQ              = board.kingSq(c);
    const Bitboard kingMask       = Bitboard::fromSquare(kSQ) & epPawnSq.rank().bb();
    const Bitboard enemyQueenRook = board.pieces(PieceType::ROOK, PieceType::QUEEN) & board.us(~c);

    auto epBB = attacks::pawn(~c, ep) & pawns_lr;

    // For one en passant square two pawns could potentially take there.
    while (epBB) {
        const auto from = epBB.pop();
        const auto to   = ep;

        /*
         If the pawn is pinned but the en passant square is not on the
         pin mask then the move is illegal.
        */
        if ((Bitboard::fromSquare(from) & pin_d) && !(pin_d & Bitboard::fromSquare(ep))) continue;

        const auto connectingPawns = Bitboard::fromSquare(epPawnSq) | Bitboard::fromSquare(from);

        /*
         7k/4p3/8/2KP3r/8/8/8/8 b - - 0 1
         If e7e5 there will be a potential ep square for us on e6.
         However, we cannot take en passant because that would put our king
         in check. For this scenario we check if there's an enemy rook/queen
         that would give check if the two pawns were removed.
         If that's the case then the move is illegal and we can break immediately.
        */
        const auto isPossiblePin = kingMask && enemyQueenRook;

        if (isPossiblePin && (attacks::rook(kSQ, board.occ() ^ connectingPawns) & enemyQueenRook) != 0ull) break;

        moves[i++] = Move::make<Move::ENPASSANT>(from, to);
    }

    return moves;
}

[[nodiscard]] inline Bitboard movegen::generateKnightMoves(Square sq) { return attacks::knight(sq); }

[[nodiscard]] inline Bitboard movegen::generateBishopMoves(Square sq, Bitboard pin_d, Bitboard occ_all) {
    // The Bishop is pinned diagonally thus can only move diagonally.
    if (pin_d & Bitboard::fromSquare(sq)) return attacks::bishop(sq, occ_all) & pin_d;
    return attacks::bishop(sq, occ_all);
}

[[nodiscard]] inline Bitboard movegen::generateRookMoves(Square sq, Bitboard pin_hv, Bitboard occ_all) {
    // The Rook is pinned horizontally thus can only move horizontally.
    if (pin_hv & Bitboard::fromSquare(sq)) return attacks::rook(sq, occ_all) & pin_hv;
    return attacks::rook(sq, occ_all);
}

[[nodiscard]] inline Bitboard movegen::generateQueenMoves(Square sq, Bitboard pin_d, Bitboard pin_hv,
                                                          Bitboard occ_all) {
    Bitboard moves = 0ULL;

    if (pin_d & Bitboard::fromSquare(sq))
        moves |= attacks::bishop(sq, occ_all) & pin_d;
    else if (pin_hv & Bitboard::fromSquare(sq))
        moves |= attacks::rook(sq, occ_all) & pin_hv;
    else {
        moves |= attacks::rook(sq, occ_all);
        moves |= attacks::bishop(sq, occ_all);
    }

    return moves;
}

[[nodiscard]] inline Bitboard movegen::generateKingMoves(Square sq, Bitboard seen, Bitboard movable_square) {
    return attacks::king(sq) & movable_square & ~seen;
}

template <Color::underlying c>
[[nodiscard]] inline Bitboard movegen::generateCastleMoves(const Board &board, Square sq, Bitboard seen,
                                                           Bitboard pin_hv) noexcept {
    if (!Square::back_rank(sq, c) || !board.castlingRights().has(c)) return 0ull;

    const auto rights = board.castlingRights();

    Bitboard moves = 0ull;

    for (const auto side : {Board::CastlingRights::Side::KING_SIDE, Board::CastlingRights::Side::QUEEN_SIDE}) {
        if (!rights.has(c, side)) continue;

        const auto is_king_side = side == Board::CastlingRights::Side::KING_SIDE;

        // No pieces on the castling path
        if (board.occ() & board.getCastlingPath(c, is_king_side)) continue;

        // No attacks on the king path
        const auto king_to = Square::castling_king_square(is_king_side, c);
        if (between(sq, king_to) & seen) continue;

        // Chess960: Rook is pinned on the backrank.
        const auto from_rook_bb = Bitboard::fromSquare(Square(rights.getRookFile(c, side), sq.rank()));
        if (board.chess960() && (pin_hv & board.us(board.sideToMove()) & from_rook_bb)) continue;

        moves |= from_rook_bb;
    }

    return moves;
}

template <typename T>
inline void movegen::whileBitboardAdd(Movelist &movelist, Bitboard mask, T func) {
    while (mask) {
        const Square from = mask.pop();
        auto moves        = func(from);
        while (moves) {
            const Square to = moves.pop();
            movelist.add(Move::make<Move::NORMAL>(from, to));
        }
    }
}

template <Color::underlying c, movegen::MoveGenType mt>
inline void movegen::legalmoves(Movelist &movelist, const Board &board, int pieces) {
    /*
     The size of the movelist might not
     be 0! This is done on purpose since it enables
     you to append new move types to any movelist.
    */
    auto king_sq = board.kingSq(c);

    Bitboard occ_us  = board.us(c);
    Bitboard occ_opp = board.us(~c);
    Bitboard occ_all = occ_us | occ_opp;

    Bitboard opp_empty = ~occ_us;

    const auto [checkmask, checks] = checkMask<c>(board, king_sq);
    const auto pin_hv              = pinMask<c, PieceType::ROOK>(board, king_sq, occ_opp, occ_us);
    const auto pin_d               = pinMask<c, PieceType::BISHOP>(board, king_sq, occ_opp, occ_us);

    assert(checks <= 2);

    Bitboard movable_square;

    // Slider, Knights and King moves can only go to enemy or empty squares.
    if constexpr (mt == MoveGenType::ALL)
        movable_square = opp_empty;
    else if constexpr (mt == MoveGenType::CAPTURE)
        movable_square = occ_opp;
    else  // QUIET moves
        movable_square = ~occ_all;

    if (pieces & PieceGenType::KING) {
        Bitboard seen = seenSquares<~c>(board, opp_empty);

        whileBitboardAdd(movelist, Bitboard::fromSquare(king_sq),
                         [&](Square sq) { return generateKingMoves(sq, seen, movable_square); });

        if (mt != MoveGenType::CAPTURE && checks == 0) {
            Bitboard moves_bb = generateCastleMoves<c>(board, king_sq, seen, pin_hv);

            while (moves_bb) {
                Square to = moves_bb.pop();
                movelist.add(Move::make<Move::CASTLING>(king_sq, to));
            }
        }
    }

    // Early return for double check as described earlier
    if (checks == 2) return;

    // Moves have to be on the checkmask
    movable_square &= checkmask;

    // Add the moves to the movelist.
    if (pieces & PieceGenType::PAWN) {
        generatePawnMoves<c, mt>(board, movelist, pin_d, pin_hv, checkmask, occ_opp);
    }

    if (pieces & PieceGenType::KNIGHT) {
        // Prune knights that are pinned since these cannot move.
        Bitboard knights_mask = board.pieces(PieceType::KNIGHT, c) & ~(pin_d | pin_hv);

        whileBitboardAdd(movelist, knights_mask, [&](Square sq) { return generateKnightMoves(sq) & movable_square; });
    }

    if (pieces & PieceGenType::BISHOP) {
        // Prune horizontally pinned bishops
        Bitboard bishops_mask = board.pieces(PieceType::BISHOP, c) & ~pin_hv;

        whileBitboardAdd(movelist, bishops_mask,
                         [&](Square sq) { return generateBishopMoves(sq, pin_d, occ_all) & movable_square; });
    }

    if (pieces & PieceGenType::ROOK) {
        //  Prune diagonally pinned rooks
        Bitboard rooks_mask = board.pieces(PieceType::ROOK, c) & ~pin_d;

        whileBitboardAdd(movelist, rooks_mask,
                         [&](Square sq) { return generateRookMoves(sq, pin_hv, occ_all) & movable_square; });
    }

    if (pieces & PieceGenType::QUEEN) {
        // Prune double pinned queens
        Bitboard queens_mask = board.pieces(PieceType::QUEEN, c) & ~(pin_d & pin_hv);

        whileBitboardAdd(movelist, queens_mask,
                         [&](Square sq) { return generateQueenMoves(sq, pin_d, pin_hv, occ_all) & movable_square; });
    }
}

template <movegen::MoveGenType mt>
inline void movegen::legalmoves(Movelist &movelist, const Board &board, int pieces) {
    movelist.clear();

    if (board.sideToMove() == Color::WHITE)
        legalmoves<Color::WHITE, mt>(movelist, board, pieces);
    else
        legalmoves<Color::BLACK, mt>(movelist, board, pieces);
}

template <Color::underlying c>
inline bool movegen::isEpSquareValid(const Board &board, Square ep) {
    const auto stm = board.sideToMove();

    Bitboard occ_us  = board.us(stm);
    Bitboard occ_opp = board.us(~stm);
    auto king_sq     = board.kingSq(stm);

    const auto [checkmask, checks] = movegen::checkMask<c>(board, king_sq);
    const auto pin_hv              = movegen::pinMask<c, PieceType::ROOK>(board, king_sq, occ_opp, occ_us);
    const auto pin_d               = movegen::pinMask<c, PieceType::BISHOP>(board, king_sq, occ_opp, occ_us);

    const auto pawns    = board.pieces(PieceType::PAWN, stm);
    const auto pawns_lr = pawns & ~pin_hv;
    const auto m        = movegen::generateEPMove(board, checkmask, pin_d, pawns_lr, ep, stm);
    bool found          = false;

    for (const auto &move : m) {
        if (move != Move::NO_MOVE) {
            found = true;
            break;
        }
    }

    return found;
}

[[nodiscard]] inline Bitboard movegen::between(Square sq1, Square sq2) noexcept {
    return SQUARES_BETWEEN_BB[sq1.index()][sq2.index()];
}

inline const std::array<std::array<Bitboard, 64>, 64> movegen::SQUARES_BETWEEN_BB = [] {
    attacks::initAttacks();
    return movegen::init_squares_between();
}();

}  // namespace chess

#include <istream>

namespace chess::pgn {

namespace detail {

/**
 * @brief Private class
 */
class StringBuffer {
   public:
    bool empty() const noexcept { return index_ == 0; }

    void clear() noexcept { index_ = 0; }

    std::string_view get() const noexcept { return std::string_view(buffer_.data(), index_); }

    bool add(char c) {
        if (index_ >= N) {
            return false;
        }

        buffer_[index_] = c;

        ++index_;

        return true;
    }

   private:
    // PGN String Tokens are limited to 255 characters
    static constexpr int N = 255;

    std::array<char, N> buffer_ = {};

    std::size_t index_ = 0;
};

/**
 * @brief Private class
 * @tparam BUFFER_SIZE
 */
template <std::size_t BUFFER_SIZE>
class StreamBuffer {
   private:
    static constexpr std::size_t N = BUFFER_SIZE;
    using BufferType               = std::array<char, N * N>;

   public:
    StreamBuffer(std::istream &stream) : stream_(stream) {}

    // Get the current character, skip carriage returns
    std::optional<char> some() {
        while (true) {
            if (buffer_index_ < bytes_read_) {
                const auto c = buffer_[buffer_index_];

                if (c == '\r') {
                    ++buffer_index_;
                    continue;
                }

                return c;
            }

            if (!fill()) {
                return std::nullopt;
            }
        }
    }

    // Assume that the current character is already the opening_delim
    bool skipUntil(char open_delim, char close_delim) {
        int stack = 0;

        while (true) {
            const auto ret = some();
            advance();

            if (!ret.has_value()) {
                return false;
            }

            if (*ret == open_delim) {
                ++stack;
            } else if (*ret == close_delim) {
                if (stack == 0) {
                    // Mismatched closing delimiter
                    return false;
                } else {
                    --stack;
                    if (stack == 0) {
                        // Matching closing delimiter found
                        return true;
                    }
                }
            }
        }

        // If we reach this point, there are unmatched opening delimiters
        return false;
    }

    bool fill() {
        buffer_index_ = 0;

        stream_.read(buffer_.data(), N * N);
        bytes_read_ = stream_.gcount();

        return bytes_read_ > 0;
    }

    void advance() {
        if (buffer_index_ >= bytes_read_) {
            fill();
        }

        ++buffer_index_;
    }

    char peek() {
        if (buffer_index_ + 1 >= bytes_read_) {
            return stream_.peek();
        }

        return buffer_[buffer_index_ + 1];
    }

    std::optional<char> current() {
        if (buffer_index_ >= bytes_read_) {
            return fill() ? std::optional<char>(buffer_[buffer_index_]) : std::nullopt;
        }

        return buffer_[buffer_index_];
    }

   private:
    std::istream &stream_;
    BufferType buffer_;
    std::streamsize bytes_read_   = 0;
    std::streamsize buffer_index_ = 0;
};

}  // namespace detail

/**
 * @brief Visitor interface for parsing PGN files
 */
class Visitor {
   public:
    virtual ~Visitor() {};

    /**
     * @brief When true, the current PGN will be skipped and only
     * endPgn will be called, this will also reset the skip flag to false.
     * Has to be called after startPgn.
     * @param skip
     */
    void skipPgn(bool skip) { skip_ = skip; }
    bool skip() { return skip_; }

    /**
     * @brief Called when a new PGN starts
     */
    virtual void startPgn() = 0;

    /**
     * @brief Called for each header
     * @param key
     * @param value
     */
    virtual void header(std::string_view key, std::string_view value) = 0;

    /**
     * @brief Called before the first move of a game
     */
    virtual void startMoves() = 0;

    /**
     * @brief Called for each move of a game
     * @param move
     * @param comment
     */
    virtual void move(std::string_view move, std::string_view comment) = 0;

    /**
     * @brief Called when a game ends
     */
    virtual void endPgn() = 0;

   private:
    bool skip_ = false;
};

class StreamParserError {
   public:
    enum Code {
        None,
        ExceededMaxStringLength,
        InvalidHeaderMissingClosingBracket,
        InvalidHeaderMissingClosingQuote,
        NotEnoughData
    };

    StreamParserError() : code_(None) {}

    StreamParserError(Code code) : code_(code) {}

    Code code() const { return code_; }

    bool hasError() const { return code_ != None; }

    std::string message() const {
        switch (code_) {
            case None:
                return "No error";
            case InvalidHeaderMissingClosingBracket:
                return "Invalid header: missing closing bracket";
            case InvalidHeaderMissingClosingQuote:
                return "Invalid header: missing closing quote";
            case NotEnoughData:
                return "Not enough data";
            default:
                assert(false);
                return "Unknown error";
        }
    }

    bool operator==(Code code) const { return code_ == code; }
    bool operator!=(Code code) const { return code_ != code; }
    bool operator==(const StreamParserError &other) const { return code_ == other.code_; }
    bool operator!=(const StreamParserError &other) const { return code_ != other.code_; }

    operator bool() const { return code_ != None; }

   private:
    Code code_;
};

template <std::size_t BUFFER_SIZE =
#if defined(__APPLE__) || defined(__MACH__)
              256
#elif defined(__unix__) || defined(__unix) || defined(unix)
              1024
#else
              256
#endif
          >
class StreamParser {
   public:
    StreamParser(std::istream &stream) : stream_buffer(stream) {}

    StreamParserError readGames(Visitor &vis) {
        visitor = &vis;

        if (!stream_buffer.fill()) {
            return StreamParserError::NotEnoughData;
        }

        while (auto c = stream_buffer.some()) {
            if (in_header) {
                visitor->skipPgn(false);

                if (*c == '[') {
                    visitor->startPgn();
                    pgn_end = false;

                    processHeader();

                    if (error != StreamParserError::None) {
                        return error;
                    }
                }

            } else if (in_body) {
                processBody();

                if (error != StreamParserError::None) {
                    return error;
                }
            }

            if (!dont_advance_after_body) stream_buffer.advance();
            dont_advance_after_body = false;
        }

        if (!pgn_end) {
            onEnd();
        }

        return error;
    }

   private:
    void reset_trackers() {
        header.first.clear();
        header.second.clear();

        move.clear();
        comment.clear();

        in_header = true;
        in_body   = false;
    }

    void callVisitorMoveFunction() {
        if (!move.empty()) {
            if (!visitor->skip()) visitor->move(move.get(), comment);

            move.clear();
            comment.clear();
        }
    }

    void processHeader() {
        bool backslash = false;

        while (auto c = stream_buffer.some()) {
            switch (*c) {
                // tag start
                case '[':
                    stream_buffer.advance();

                    while (auto k = stream_buffer.some()) {
                        if (is_space(*k)) {
                            break;
                        } else {
                            if (!header.first.add(*k)) {
                                error = StreamParserError::ExceededMaxStringLength;
                                return;
                            }

                            stream_buffer.advance();
                        }
                    }

                    stream_buffer.advance();
                    break;
                case '"':
                    stream_buffer.advance();

                    while (auto k = stream_buffer.some()) {
                        if (*k == '\\') {
                            backslash = true;
                            // don't add backslash to header, is this really correct?
                            stream_buffer.advance();
                        } else if (*k == '"' && !backslash) {
                            stream_buffer.advance();

                            // we should be now at ]
                            if (stream_buffer.current().value_or('\0') != ']') {
                                error = StreamParserError::InvalidHeaderMissingClosingBracket;
                                return;
                            }

                            stream_buffer.advance();

                            break;
                        } else if (*k == '\n') {
                            // we missed the closing quote and read until the newline character
                            // this is an invalid pgn, let's throw an error
                            error = StreamParserError::InvalidHeaderMissingClosingQuote;
                            return;
                        } else {
                            backslash = false;

                            if (!header.second.add(*k)) {
                                error = StreamParserError::ExceededMaxStringLength;
                                return;
                            }

                            stream_buffer.advance();
                        }
                    }

                    // manually skip carriage return, otherwise we would be in the body
                    // ideally we should completely skip all carriage returns and newlines to avoid this
                    if (stream_buffer.current() == '\r') {
                        stream_buffer.advance();
                    }

                    if (!visitor->skip()) visitor->header(header.first.get(), header.second.get());

                    header.first.clear();
                    header.second.clear();

                    stream_buffer.advance();
                    break;
                case '\n':
                    in_header = false;
                    in_body   = true;

                    if (!visitor->skip()) visitor->startMoves();

                    return;
                default:
                    // this should normally not happen
                    // lets just go into the body, will this always be save?
                    in_header = false;
                    in_body   = true;

                    if (!visitor->skip()) visitor->startMoves();

                    return;
            }
        }
    }

    void processBody() {
        auto is_termination_symbol = false;
        auto has_comment           = false;

    start:
        /*
        Skip first move number or game termination
        Also skip - * / to fix games
        which directly start with a game termination
        this https://github.com/Disservin/chess-library/issues/68
        */

        while (auto c = stream_buffer.some()) {
            if (*c == ' ' || is_digit(*c)) {
                stream_buffer.advance();
            } else if (*c == '-' || *c == '*' || *c == '/') {
                is_termination_symbol = true;
                stream_buffer.advance();
            } else if (*c == '{') {
                has_comment = true;

                // reading comment
                stream_buffer.advance();

                while (auto k = stream_buffer.some()) {
                    stream_buffer.advance();

                    if (*k == '}') {
                        break;
                    }

                    comment += *k;
                }

                // the game has no moves, but a comment followed by a game termination
                if (!visitor->skip()) {
                    visitor->move("", comment);

                    has_comment = false;
                    comment.clear();
                }
            } else {
                break;
            }
        }

        // we need to reparse the termination symbol
        if (!visitor->skip() && has_comment && !is_termination_symbol) {
            goto start;
        }

        // game had no moves, so we can skip it and call endPgn
        if (is_termination_symbol) {
            onEnd();
            return;
        }

        while (auto c = stream_buffer.some()) {
            if (is_space(*c)) {
                stream_buffer.advance();
                continue;
            }

            break;
        }

        while (auto cd = stream_buffer.some()) {
            // Pgn are build up in the following way.
            // {move_number} {move} {comment} {move} {comment} {move_number} ...
            // So we need to skip the move_number then start reading the move, then save the comment
            // then read the second move in the group. After that a move_number will follow again.

            // [ is unexpected here, it probably is a new pgn and the current one is finished
            if (*cd == '[') {
                onEnd();
                dont_advance_after_body = true;
                // break;
                break;
            }

            // skip move number digits
            while (auto c = stream_buffer.some()) {
                if (is_space(*c) || is_digit(*c)) {
                    stream_buffer.advance();
                } else {
                    break;
                }
            }

            // skip dots
            while (auto c = stream_buffer.some()) {
                if (*c == '.') {
                    stream_buffer.advance();
                } else {
                    break;
                }
            }

            // skip spaces
            while (auto c = stream_buffer.some()) {
                if (is_space(*c)) {
                    stream_buffer.advance();
                } else {
                    break;
                }
            }

            // parse move
            if (parseMove()) {
                break;
            }

            // skip spaces
            while (auto c = stream_buffer.some()) {
                if (is_space(*c)) {
                    stream_buffer.advance();
                } else {
                    break;
                }
            }

            // game termination
            auto curr = stream_buffer.current();

            if (!curr.has_value()) {
                onEnd();
                break;
            }

            // game termination
            if (*curr == '*') {
                onEnd();
                stream_buffer.advance();

                break;
            }

            const auto peek = stream_buffer.peek();

            if (*curr == '1') {
                if (peek == '-') {
                    stream_buffer.advance();
                    stream_buffer.advance();

                    onEnd();
                    break;
                } else if (peek == '/') {
                    for (size_t i = 0; i <= 6; ++i) {
                        stream_buffer.advance();
                    }

                    onEnd();
                    break;
                }
            }

            // might be 0-1 (game termination) or 0-0-0/0-0 (castling)
            if (*curr == '0' && stream_buffer.peek() == '-') {
                stream_buffer.advance();
                stream_buffer.advance();

                const auto c = stream_buffer.current();
                if (!c.has_value()) {
                    onEnd();

                    break;
                }

                // game termination
                if (*c == '1') {
                    onEnd();
                    stream_buffer.advance();

                    break;
                }
                // castling
                else {
                    if (!move.add('0') || !move.add('-')) {
                        error = StreamParserError::ExceededMaxStringLength;
                        return;
                    }

                    if (parseMove()) {
                        stream_buffer.advance();
                        break;
                    }
                }
            }
        }
    }

    bool parseMove() {
        // reading move
        while (auto c = stream_buffer.some()) {
            if (is_space(*c)) {
                break;
            }

            if (!move.add(*c)) {
                error = StreamParserError::ExceededMaxStringLength;
                return true;
            }

            stream_buffer.advance();
        }

        return parseMoveAppendix();
    }

    bool parseMoveAppendix() {
        while (true) {
            auto curr = stream_buffer.current();

            if (!curr.has_value()) {
                onEnd();
                return true;
            }

            switch (*curr) {
                case '{': {
                    // reading comment
                    stream_buffer.advance();

                    while (auto c = stream_buffer.some()) {
                        stream_buffer.advance();

                        if (*c == '}') {
                            break;
                        }

                        comment += *c;
                    }

                    break;
                }
                case '(': {
                    stream_buffer.skipUntil('(', ')');
                    break;
                }
                case '$': {
                    while (auto c = stream_buffer.some()) {
                        if (is_space(*c)) {
                            break;
                        }

                        stream_buffer.advance();
                    }

                    break;
                }
                case ' ': {
                    while (auto c = stream_buffer.some()) {
                        if (!is_space(*c)) {
                            break;
                        }

                        stream_buffer.advance();
                    }

                    break;
                }
                default:
                    callVisitorMoveFunction();
                    return false;
            }
        }
    }

    void onEnd() {
        callVisitorMoveFunction();
        visitor->endPgn();
        visitor->skipPgn(false);

        reset_trackers();

        pgn_end = true;
    }

    bool is_space(const char c) noexcept {
        switch (c) {
            case ' ':
            case '\t':
            case '\n':
            case '\r':
                return true;
            default:
                return false;
        }
    }

    bool is_digit(const char c) noexcept {
        switch (c) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                return true;
            default:
                return false;
        }
    }

    detail::StreamBuffer<BUFFER_SIZE> stream_buffer;

    Visitor *visitor = nullptr;

    // one time allocations
    std::pair<detail::StringBuffer, detail::StringBuffer> header = {detail::StringBuffer{}, detail::StringBuffer{}};

    detail::StringBuffer move = {};
    std::string comment       = {};

    // State

    StreamParserError error = StreamParserError::None;

    bool in_header = true;
    bool in_body   = false;

    bool pgn_end = true;

    bool dont_advance_after_body = false;
};
}  // namespace chess::pgn

#include <sstream>


namespace chess {
class uci {
   public:
    /**
     * @brief Converts an internal move to a UCI string
     * @param move
     * @param chess960
     * @return
     */
    [[nodiscard]] static std::string moveToUci(const Move &move, bool chess960 = false) noexcept(false) {
        // Get the from and to squares
        Square from_sq = move.from();
        Square to_sq   = move.to();

        // If the move is not a chess960 castling move and is a king moving more than one square,
        // update the to square to be the correct square for a regular castling move
        if (!chess960 && move.typeOf() == Move::CASTLING) {
            to_sq = Square(to_sq > from_sq ? File::FILE_G : File::FILE_C, from_sq.rank());
        }

        std::stringstream ss;

        // Add the from and to squares to the string stream
        ss << from_sq << to_sq;

        // If the move is a promotion, add the promoted piece to the string stream
        if (move.typeOf() == Move::PROMOTION) {
            ss << static_cast<std::string>(move.promotionType());
        }

        return ss.str();
    }

    /**
     * @brief Converts a UCI string to an internal move.
     * @param board
     * @param uci
     * @return
     */
    [[nodiscard]] static Move uciToMove(const Board &board, const std::string &uci) noexcept(false) {
        if (uci.length() < 4) {
            return Move::NO_MOVE;
        }

        Square source = Square(uci.substr(0, 2));
        Square target = Square(uci.substr(2, 2));

        if (!source.is_valid() || !target.is_valid()) {
            return Move::NO_MOVE;
        }

        auto pt = board.at(source).type();

        // castling in chess960
        if (board.chess960() && pt == PieceType::KING && board.at(target).type() == PieceType::ROOK &&
            board.at(target).color() == board.sideToMove()) {
            return Move::make<Move::CASTLING>(source, target);
        }

        // convert to king captures rook
        // in chess960 the move should be sent as king captures rook already!
        if (!board.chess960() && pt == PieceType::KING && Square::distance(target, source) == 2) {
            target = Square(target > source ? File::FILE_H : File::FILE_A, source.rank());
            return Move::make<Move::CASTLING>(source, target);
        }

        // en passant
        if (pt == PieceType::PAWN && target == board.enpassantSq()) {
            return Move::make<Move::ENPASSANT>(source, target);
        }

        // promotion
        if (pt == PieceType::PAWN && uci.length() == 5 && Square::back_rank(target, ~board.sideToMove())) {
            auto promotion = PieceType(uci.substr(4, 1));

            if (promotion == PieceType::NONE || promotion == PieceType::KING || promotion == PieceType::PAWN) {
                return Move::NO_MOVE;
            }

            return Move::make<Move::PROMOTION>(source, target, PieceType(uci.substr(4, 1)));
        }

        return (uci.length() == 4) ? Move::make<Move::NORMAL>(source, target) : Move::NO_MOVE;
    }

    /**
     * @brief Converts a move to a SAN string
     * @param board
     * @param move
     * @return
     */
    [[nodiscard]] static std::string moveToSan(const Board &board, const Move &move) noexcept(false) {
        std::string san;
        moveToRep<false>(board, move, san);
        return san;
    }

    /**
     * @brief Converts a move to a LAN string
     * @param board
     * @param move
     * @return
     */
    [[nodiscard]] static std::string moveToLan(const Board &board, const Move &move) noexcept(false) {
        std::string lan;
        moveToRep<true>(board, move, lan);
        return lan;
    }

    class SanParseError : public std::exception {
       public:
        explicit SanParseError(const char *message) : msg_(message) {}

        explicit SanParseError(const std::string &message) : msg_(message) {}

        virtual ~SanParseError() noexcept {}

        virtual const char *what() const noexcept { return msg_.c_str(); }

       protected:
        std::string msg_;
    };

    class AmbiguousMoveError : public std::exception {
       public:
        explicit AmbiguousMoveError(const char *message) : msg_(message) {}

        explicit AmbiguousMoveError(const std::string &message) : msg_(message) {}

        virtual ~AmbiguousMoveError() noexcept {}

        virtual const char *what() const noexcept { return msg_.c_str(); }

       protected:
        std::string msg_;
    };

    /**
     * @brief Parse a san string and return the move.
     * This function will throw a SanParseError if the san string is invalid.
     * @param board
     * @param san
     * @return
     */
    [[nodiscard]] static Move parseSan(const Board &board, std::string_view san) noexcept(false) {
        Movelist moves;

        return parseSan(board, san, moves);
    }

    /**
     * @brief Parse a san string and return the move.
     * This function will throw a SanParseError if the san string is invalid.
     * @param board
     * @param san
     * @param moves
     * @return
     */
    [[nodiscard]] static Move parseSan(const Board &board, std::string_view san, Movelist &moves) noexcept(false) {
        if (san.empty()) {
            return Move::NO_MOVE;
        }

        static constexpr auto pt_to_pgt = [](PieceType pt) { return 1 << (pt); };
        const SanMoveInformation info   = parseSanInfo(san);

        if (info.capture) {
            movegen::legalmoves<movegen::MoveGenType::CAPTURE>(moves, board, pt_to_pgt(info.piece));
        } else {
            movegen::legalmoves<movegen::MoveGenType::QUIET>(moves, board, pt_to_pgt(info.piece));
        }

        if (info.castling_short || info.castling_long) {
            for (const auto &move : moves) {
                if (move.typeOf() == Move::CASTLING) {
                    if ((info.castling_short && move.to() > move.from()) ||
                        (info.castling_long && move.to() < move.from())) {
                        return move;
                    }
                }
            }

#ifndef CHESS_NO_EXCEPTIONS
            throw SanParseError("Failed to parse san. At step 2: " + std::string(san) + " " + board.getFen());
#endif
        }

        Move matchingMove = Move::NO_MOVE;
        bool foundMatch   = false;

        for (const auto &move : moves) {
            // Skip all moves that are not to the correct square
            // or are castling moves
            if (move.to() != info.to || move.typeOf() == Move::CASTLING) {
                continue;
            }

            // Handle promotion moves
            if (info.promotion != PieceType::NONE) {
                if (move.typeOf() != Move::PROMOTION || info.promotion != move.promotionType() ||
                    move.from().file() != info.from_file) {
                    continue;
                }
            }
            // Handle en passant moves
            else if (move.typeOf() == Move::ENPASSANT) {
                if (move.from().file() != info.from_file) {
                    continue;
                }
            }
            // Handle moves with specific from square
            else if (info.from != Square::NO_SQ) {
                if (move.from() != info.from) {
                    continue;
                }
            }
            // Handle moves with partial from information (rank or file)
            else if (info.from_rank != Rank::NO_RANK || info.from_file != File::NO_FILE) {
                if ((info.from_file != File::NO_FILE && move.from().file() != info.from_file) ||
                    (info.from_rank != Rank::NO_RANK && move.from().rank() != info.from_rank)) {
                    continue;
                }
            }

            // If we get here, the move matches our criteria
            if (foundMatch) {
#ifndef CHESS_NO_EXCEPTIONS
                throw AmbiguousMoveError("Ambiguous san: " + std::string(san) + " in " + board.getFen());
#endif
            }

            matchingMove = move;
            foundMatch   = true;
        }

        if (!foundMatch) {
#ifndef CHESS_NO_EXCEPTIONS
            throw SanParseError("Failed to parse san, illegal move: " + std::string(san) + " " + board.getFen());
#endif
        }

        return matchingMove;
    }

    /**
     * @brief Check if a string is a valid UCI move. Must also have the correct length.
     * @param move
     * @return
     */
    static bool isUciMove(const std::string &move) noexcept {
        bool is_uci = false;

        static constexpr auto is_digit     = [](char c) { return c >= '1' && c <= '8'; };
        static constexpr auto is_file      = [](char c) { return c >= 'a' && c <= 'h'; };
        static constexpr auto is_promotion = [](char c) { return c == 'n' || c == 'b' || c == 'r' || c == 'q'; };

        // assert that the move is in uci format, [abcdefgh][1-8][abcdefgh][1-8][nbrq]
        if (move.size() >= 4) {
            is_uci = is_file(move[0]) && is_digit(move[1]) && is_file(move[2]) && is_digit(move[3]);
        }

        if (move.size() == 5) {
            is_uci = is_uci && is_promotion(move[4]);
        }

        if (move.size() > 5) {
            return false;
        }

        return is_uci;
    }

   private:
    struct SanMoveInformation {
        File from_file = File::NO_FILE;
        Rank from_rank = Rank::NO_RANK;

        PieceType promotion = PieceType::NONE;

        Square from = Square::NO_SQ;
        // a valid move always has a to square
        Square to = Square::NO_SQ;

        // a valid move always has a piece
        PieceType piece = PieceType::NONE;

        bool castling_short = false;
        bool castling_long  = false;

        bool capture = false;
    };

    [[nodiscard]] static SanMoveInformation parseSanInfo(std::string_view san) noexcept(false) {
#ifndef CHESS_NO_EXCEPTIONS
        if (san.length() < 2) {
            throw SanParseError("Failed to parse san. At step 0: " + std::string(san));
        }
#endif
        constexpr auto parse_castle = [](std::string_view &san, SanMoveInformation &info, char castling_char) {
            info.piece = PieceType::KING;

            san.remove_prefix(3);

            info.castling_short = san.length() == 0 || (san.length() >= 1 && san[0] != '-');
            info.castling_long  = san.length() >= 2 && san[0] == '-' && san[1] == castling_char;

            assert((info.castling_short && !info.castling_long) || (!info.castling_short && info.castling_long) ||
                   (!info.castling_short && !info.castling_long));
        };

        static constexpr auto isRank = [](char c) { return c >= '1' && c <= '8'; };
        static constexpr auto isFile = [](char c) { return c >= 'a' && c <= 'h'; };
        static constexpr auto sw     = [](const char &c) { return std::string_view(&c, 1); };

        SanMoveInformation info;
        bool throw_error = false;

        // set to 1 to skip piece type offset
        std::size_t index = 1;

        if (san[0] == 'O' || san[0] == '0') {
            parse_castle(san, info, san[0]);
            return info;
        } else if (isFile(san[0])) {
            index--;
            info.piece = PieceType::PAWN;
        } else {
            info.piece = PieceType(san);
            if (info.piece == PieceType::NONE) {
                throw_error = true;
            }
        }

        File file_to = File::NO_FILE;
        Rank rank_to = Rank::NO_RANK;

        // check if san starts with a file, if so it will be start file
        if (index < san.size() && isFile(san[index])) {
            info.from_file = File(sw(san[index]));
            index++;
        }

        // check if san starts with a rank, if so it will be start rank
        if (index < san.size() && isRank(san[index])) {
            info.from_rank = Rank(sw(san[index]));
            index++;
        }

        // skip capture sign
        if (index < san.size() && san[index] == 'x') {
            info.capture = true;
            index++;
        }

        // to file
        if (index < san.size() && isFile(san[index])) {
            file_to = File(sw(san[index]));
            index++;
        }

        // to rank
        if (index < san.size() && isRank(san[index])) {
            rank_to = Rank(sw(san[index]));
            index++;
        }

        // promotion
        if (index < san.size() && san[index] == '=') {
            index++;
            info.promotion = PieceType(sw(san[index]));
            if (info.promotion == PieceType::KING || info.promotion == PieceType::PAWN ||
                info.promotion == PieceType::NONE) {
                throw_error = true;
            }
            index++;
        }

        // for simple moves like Nf3, e4, etc. all the information is contained
        // in the from file and rank. Thus we need to move it to the to file and rank.
        if (file_to == File::NO_FILE && rank_to == Rank::NO_RANK) {
            file_to = info.from_file;
            rank_to = info.from_rank;

            info.from_file = File::NO_FILE;
            info.from_rank = Rank::NO_RANK;
        }

        // pawns which are not capturing stay on the same file
        if (info.piece == PieceType::PAWN && info.from_file == File::NO_FILE && !info.capture) {
            info.from_file = file_to;
        }

        if (file_to != File::NO_FILE && rank_to != Rank::NO_RANK) {
            info.to = Square(file_to, rank_to);
        } else {
            throw_error = true;
        }

#ifndef CHESS_NO_EXCEPTIONS
        if (throw_error) {
            throw SanParseError("Failed to parse san. At step 1: " + std::string(san));
        }
#endif

        if (info.from_file != File::NO_FILE && info.from_rank != Rank::NO_RANK) {
            info.from = Square(info.from_file, info.from_rank);
        }

        return info;
    }

    template <bool LAN = false>
    static void moveToRep(Board board, const Move &move, std::string &str) {
        if (handleCastling(move, str)) {
            board.makeMove(move);
            if (board.inCheck()) appendCheckSymbol(board, str);
            return;
        }

        const PieceType pt   = board.at(move.from()).type();
        const bool isCapture = board.at(move.to()) != Piece::NONE || move.typeOf() == Move::ENPASSANT;

        assert(pt != PieceType::NONE);

        if (pt != PieceType::PAWN) {
            appendPieceSymbol(pt, str);
        }

        if constexpr (LAN) {
            appendSquare(move.from(), str);
        } else {
            if (pt == PieceType::PAWN) {
                str += isCapture ? static_cast<std::string>(move.from().file()) : "";
            } else {
                resolveAmbiguity(board, move, pt, str);
            }
        }

        if (isCapture) {
            str += 'x';
        }

        appendSquare(move.to(), str);

        if (move.typeOf() == Move::PROMOTION) appendPromotion(move, str);

        board.makeMove(move);

        if (board.inCheck()) appendCheckSymbol(board, str);
    }

    static bool handleCastling(const Move &move, std::string &str) {
        if (move.typeOf() != Move::CASTLING) return false;

        str = (move.to().file() > move.from().file()) ? "O-O" : "O-O-O";
        return true;
    }

    static void appendPieceSymbol(PieceType pieceType, std::string &str) {
        str += std::toupper(static_cast<std::string>(pieceType)[0]);
    }

    static void appendSquare(Square square, std::string &str) {
        str += static_cast<std::string>(square.file());
        str += static_cast<std::string>(square.rank());
    }

    static void appendPromotion(const Move &move, std::string &str) {
        str += '=';
        str += std::toupper(static_cast<std::string>(move.promotionType())[0]);
    }

    static void appendCheckSymbol(Board &board, std::string &str) {
        const auto gameState = board.isGameOver().second;
        str += (gameState == GameResult::LOSE) ? '#' : '+';
    }

    static void resolveAmbiguity(const Board &board, const Move &move, PieceType pieceType, std::string &str) {
        Movelist moves;
        movegen::legalmoves(moves, board, 1 << pieceType);

        bool needFile         = false;
        bool needRank         = false;
        bool hasAmbiguousMove = false;

        for (const auto &m : moves) {
            if (m != move && m.to() == move.to()) {
                hasAmbiguousMove = true;

                /*
                First, if the moving pieces can be distinguished by their originating files, the originating
                file letter of the moving piece is inserted immediately after the moving piece letter.

                Second (when the first step fails), if the moving pieces can be distinguished by their
                originating ranks, the originating rank digit of the moving piece is inserted immediately after
                the moving piece letter.

                Third (when both the first and the second steps fail), the two character square coordinate of
                the originating square of the moving piece is inserted immediately after the moving piece
                letter.
                */

                if (isIdentifiableByType(moves, move, move.from().file())) {
                    needFile = true;
                    break;
                }

                if (isIdentifiableByType(moves, move, move.from().rank())) {
                    needRank = true;
                    break;
                }
            }
        }

        if (needFile) str += static_cast<std::string>(move.from().file());
        if (needRank) str += static_cast<std::string>(move.from().rank());

        // we weren't able to disambiguate the move by either file or rank, so we need to use both
        if (hasAmbiguousMove && !needFile && !needRank) {
            appendSquare(move.from(), str);
        }
    }

    template <typename CoordinateType>
    static bool isIdentifiableByType(const Movelist &moves, const Move move, CoordinateType type) {
        static_assert(std::is_same_v<CoordinateType, File> || std::is_same_v<CoordinateType, Rank>,
                      "CoordinateType must be either File or Rank");

        for (const auto &m : moves) {
            if (m == move || m.to() != move.to()) {
                continue;
            }

            // file
            if constexpr (std::is_same_v<CoordinateType, File>) {
                if (type == m.from().file()) return false;
            }
            // rank
            else {
                if (type == m.from().rank()) return false;
            }
        }

        return true;
    }
};
}  // namespace chess

#endif