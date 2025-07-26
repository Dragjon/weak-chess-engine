#include "chess.hpp"
#include "see.hpp"
#include "defaults.hpp"

using namespace chess;
using namespace std;

int32_t see_piece_values[7] = {105, 340, 312, 502, 928, 0, 0};

// Estimate the value of a move
int32_t move_estimated_value(Board &board, Move move){

    // Value of piece on target square
    int32_t value = see_piece_values[board.at(move.to()).type()];
    
    uint16_t type = move.typeOf();

    // Promotion
    if (type == Move::PROMOTION)
        value += see_piece_values[move.promotionType()] - see_piece_values[0];

    // En Passant moves
    else if (type == Move::ENPASSANT)
        value = see_piece_values[0];

    // I'm not sure of how the library encodes castling moves
    // so I will just have this as a safety
    else if (type == Move::CASTLING) {
        value = 0;
    }

    return value;
}

Bitboard all_attackers_to_square(Board &board, Bitboard occ, Square sq){
    return (attacks::pawn(Color::WHITE, sq) & board.us(Color::BLACK) & board.pieces(PieceType::PAWN))
        |  (attacks::pawn(Color::BLACK, sq) & board.us(Color::WHITE) & board.pieces(PieceType::PAWN))
        |  (attacks::knight(sq) & board.pieces(PieceType::KNIGHT))
        |  (attacks::bishop(sq, occ) & (board.pieces(PieceType::BISHOP) | board.pieces(PieceType::QUEEN)))
        |  (attacks::rook(sq, occ) & (board.pieces(PieceType::ROOK) | board.pieces(PieceType::QUEEN)))
        |  (attacks::king(sq) & board.pieces(PieceType::KING));
}

// Static Exchange Evluation
// https://github.com/AndyGrant/Ethereal/blob/0e47e9b67f345c75eb965d9fb3e2493b6a11d09a/src/search.c#L929
bool see(Board board, Move move, int32_t threshold){
    int32_t balance, from, to, next_victim;
    uint16_t type;
    Color turn = board.sideToMove();
    Bitboard bishops, rooks, occupied, attackers, my_attackers;

    from = move.from().index();
    to = move.to().index();
    type = move.typeOf();

    next_victim = type != Move::PROMOTION ? static_cast<int32_t>(board.at(static_cast<Square>(from)).type()) : static_cast<int32_t>(move.promotionType());
    
    balance = move_estimated_value(board, move) - threshold;

    if (balance < 0) return false;

    // Worst case is losing the moved piece
    balance -= see_piece_values[next_victim];

    // If balance is positive even after losing moved piece, exchange is guranteed
    // to be winning
    if (balance >= 0) return true;

    bishops = board.pieces(PieceType::BISHOP) | board.pieces(PieceType::QUEEN);
    rooks = board.pieces(PieceType::ROOK) | board.pieces(PieceType::QUEEN);

    occupied = board.occ();
    occupied = (occupied ^ (1ull << from)) | (1ull << to);
    if (type == Move::ENPASSANT)
        occupied ^= (1ull << board.enpassantSq().index());

    attackers = all_attackers_to_square(board, occupied, static_cast<Square>(to)) & occupied;

    turn = turn == Color::WHITE ? Color::BLACK : Color::WHITE;

    while (true){
        my_attackers = attackers & board.us(turn);
        if (my_attackers.getBits() == 0ull){
            break;
        }

        for (next_victim = 0; next_victim <= 4; next_victim++)
            if (my_attackers & board.pieces(PieceType(static_cast<PieceType::underlying>(next_victim)))){
                break;
            }

        occupied ^= (1ull << (my_attackers & board.pieces(PieceType(static_cast<PieceType::underlying>(next_victim)))).lsb());

        if (next_victim == 0 || next_victim == 2 || next_victim == 4)
            attackers |= attacks::bishop(static_cast<Square>(to), occupied) & bishops;
        
        if (next_victim == 3 || next_victim == 4)
            attackers |= attacks::rook(static_cast<Square>(to), occupied) & rooks;

        attackers &= occupied;

        turn = turn == Color::WHITE ? Color::BLACK : Color::WHITE;

        balance = -balance - 1 - see_piece_values[next_victim];

        if (balance >= 0) {
            if (next_victim == 5 && (attackers & board.us(turn)))
                turn = turn == Color::WHITE ? Color::BLACK : Color::WHITE;
            break;
        }
        
    }

    return board.sideToMove() != turn;
    
}