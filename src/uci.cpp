// Dev command for engine testing
// fastchess.exe -engine cmd=C:\Users\dragon\Desktop\weak-dev\src\win-x64\weak.exe name=WeakDev -engine cmd=C:\Users\dragon\Desktop\weak-chess-engine\src\win-x64\weak.exe name=Weak -each tc=8+0.08 -rounds 1000000 -repeat -concurrency 64 -sprt elo0=0 elo1=5 alpha=0.05 beta=0.05 -openings file=C:\Users\dragon\Downloads\8movesv3.pgn format=pgn plies=16 --force-concurrency
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "chess.hpp"
#include "uci.hpp"
#include "timeman.hpp"
#include "eval.hpp"
#include "search.hpp"
#include "transposition.hpp"

using namespace std;
using namespace chess;

// Process UCI Commands
// For basic SPRT functionality, we only need to implement these

///////////////// At the start ////////////////////
// gui-to-engine> uci
// engine-to-gui> id name <engine name>
// engine-to-gui> id author <your name>
// engine-to-gui> uciok
// gui-to-engine> isready
// engine-to-gui> readyok**

//////////////// During the game /////////////////
// gui-to-engine> position (startpos | fen <fen>) [moves <move1> <move2> ...]
// eg. position startpos
// eg. position startpos moves e2e4 e7e5 glf3
// eg. position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1
// eg. position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 moves d5e6 a6e2 c3e2
// <wtime> - white time in ms
// <btime> - black time in ms
// <winc> - White increment in ms
// <binc> - Black increment in ms
// <movestogo> - doesn't matter (unless your chess engine uses it fot time management)
// gui-to-engine> go wtime <wtime> btime <btime> [winc <winc>] [binc <binc>] [movestogo <movestogo>]
// eg. go wtime 300000 btime 300000 winc 2000 binc 2000
// engine-to-gui> [info depth ...] // Optional
// engine-to-gui> bestmove <best move in uci notation> 

///////////////// After the game ////////////////////
// gui-to-engine> quit // quit the engine

// Full uci spec https://backscattering.de/chess/uci/

Board board = Board(STARTPOS_FEN);

// Prints the board, nothing else
void print_board(const Board &board){
    int display_board[64]{};
    // Apparently Disservin's chess library doesn't have a built in print
    // board function so we need to implement one ourselves. Here, we get
    // all the pieces bitboards so that we can get the squares of each
    // piece later through lsb()
    Bitboard wp = board.pieces(PieceType::PAWN, Color::WHITE);
    Bitboard wn = board.pieces(PieceType::KNIGHT, Color::WHITE);
    Bitboard wb = board.pieces(PieceType::BISHOP, Color::WHITE);
    Bitboard wr = board.pieces(PieceType::ROOK, Color::WHITE);
    Bitboard wq = board.pieces(PieceType::QUEEN, Color::WHITE);
    Bitboard wk = board.pieces(PieceType::KING, Color::WHITE);
    Bitboard bp = board.pieces(PieceType::PAWN, Color::BLACK);
    Bitboard bn = board.pieces(PieceType::KNIGHT, Color::BLACK);
    Bitboard bb = board.pieces(PieceType::BISHOP, Color::BLACK);
    Bitboard br = board.pieces(PieceType::ROOK, Color::BLACK);
    Bitboard bq = board.pieces(PieceType::QUEEN, Color::BLACK);
    Bitboard bk = board.pieces(PieceType::KING, Color::BLACK);
    Bitboard all[] = {wp, wn, wb, wr, wq, wk, bp, bn, bb, br, bq, bk};

    // Filling up the display_board for later use during printing. Indeed
    // a mailbox may have been easier for such a task but whatever I 
    // appreciate the speed :/
    for (int i = 0; i < 12; i++){
        Bitboard current_bb = all[i];
        while (current_bb){
            int square = current_bb.pop();
            // We do i+1 instead of i becuase we need to distinguish between white pawns
            // and empty squares when printing since we initialised display_board with 
            // all 0s. We need to ^56 because we are seeing from white's perspective.
            // ^56 basically just reverses the ranks but keeps the files the same.
            display_board[square^56] = i+1;
        }
    }

    // Here is where we are actually printing the board :) Yay! but the
    // horrendous switch statements required will ruin anyone's day
    for (int i = 0; i < 64; i++){

        // To format the board to look like a chess board, we need to endline
        // every time a chess row ends, ie after the "h" file
        if (i != 0 && i % 8 == 0)
            cout << "\n";

        string piece_char = ".";

        // This, folks, is a switch statement
        // We need to do display_board[i]-1 because all pieces are offsetted by when we
        // are indexing above using the bitboard loop
        switch(display_board[i]-1){
            case 0:
                piece_char = "P";
                break;
            case 1:
                piece_char = "N";
                break;
            case 2:
                piece_char = "B";
                break;
            case 3:
                piece_char = "R";
                break;
            case 4:
                piece_char = "Q";
                break;
            case 5:
                piece_char = "K";
                break;
            case 6:
                piece_char = "p";
                break;
            case 7:
                piece_char = "n";
                break;
            case 8:
                piece_char = "b";
                break;
            case 9:
                piece_char = "r";
                break;
            case 10:
                piece_char = "q";
                break;
            case 11:
                piece_char = "k";
                break;

        }

        cout << piece_char << " ";
    }
    cout << "\n";
}

// Main UCI loop
int main() {

    string input;
    
    // While loop for input
    while (true) {
        
        // Get the current input
        getline(cin, input);

        // We split the string into a vector by spaces for easy access
        stringstream ss(input);
        vector<string> words;
        string word;

        while (ss >> word) {
            words.push_back(word);
        }

        // Tell the GUI our engine name and author when prompted with
        // "uci". Additionally we can inform the GUI about our options
        // or parameters eg. Hash & Threads which are the most basic
        // ones. After which we must respond with "uciok"
        if (words[0] == "uci"){
            cout << "id name " << ENGINE_NAME << "-" << ENGINE_VERSION << "\n";
            cout << "id author " << ENGINE_AUTHOR << "\n";
            cout << "uciok\n";
        }

        // When the GUI prompts our engine with "isready", usually after
        // "ucinewgame" or when our engine crashes/doesn't respond for
        // a long while. Otherwise, "isready" is called right after the 
        // first "uci" to our engine. We must always reply with "readyok"
        // to "isready".
        else if (words[0] == "isready")
            cout << "readyok\n";

        else if (words[0] == "ucinewgame"){
            tt.clear();
        }

        // Parse the position command. The position commands comes in a number
        // of forms, particularly "position startpos", "position startpos moves
        // ...", "position fen <fen>", "position fen <fen> moves ...". Note the
        // moves are in UCI notation
        else if (words[0] == "position"){
            int next_idx = 2;
            bool writing_moves = false;
            if (words[1] == "startpos"){
                board = Board(STARTPOS_FEN);

                // If there is more than just startpos, there must be extra info ie moves
                // we need to parse.
                if (words.size() > 2){
                    next_idx++;
                    writing_moves = true;
                }
            }
            else if (words[1] == "fen"){
                string fen = "";
                while (true){
                    fen += words[next_idx] + " ";
                    next_idx++;

                    // If it exceeds out words size, this means that we do not have any more room left
                    // for moves. we break out of the loop, leaving writing_moves false. 
                    if (next_idx == words.size()){
                        board = Board(fen);
                        break;
                    }

                    // When we encounter a "moves" string, we stop parsing the fen and start writing
                    // moves, we set the writing_moves variable true
                    if (words[next_idx] == "moves"){
                        next_idx++;
                        writing_moves = true;
                        board = Board(fen);
                        break;
                    }
                }
            }

            // We make all the moves on the board given by the position string
            while (writing_moves) {
                board.makeMove(uci::uciToMove(board, words[next_idx]));
                next_idx++;
                // If it exceeds out words size, this means that we do not have any moves left to make.
                // We can stop parsing moves here
                if (next_idx == words.size())
                    break;
            }

        }

        // Handle the "go" command from the GUI. This can come in many forms. Normally, we only need
        // to handle "go infinite" or "go wtime <wtime> btime <btime> winc <winc> binc <binc>" in
        // any order. Some strange people have come up with strange tms like "movenumber" or 
        // "movetime" or whatever. Who cares? We just need wtime and btime for our super simple
        // time management. We don't even need increment!
        else if (words[0] == "go"){
            global_depth = 0;
            total_nodes = 0;
            max_hard_time_ms = 10000;
            max_soft_time_ms = 30000;
            reset_killers();
            reset_quiet_history();
            if (words.size() > 1){
                if (words[1] == "infinite"){
                    max_hard_time_ms = 10000000000;
                    max_soft_time_ms = 10000000000;
                }
                else {
                    for (int i = 1; i + 1 < words.size(); i+=2){
                        // If its white to move we get white's time else we get black's time
                        if (board.sideToMove() == Color::WHITE && words[i] == "wtime"){
                            max_hard_time_ms = std::stoll(words[i+1]) / HARD_TM_RATIO;
                            max_soft_time_ms = std::stoll(words[i+1]) / SOFT_TM_RATIO;
                            // Immediately break up of the loop to not waste time
                            break;
                        }
                        else if (board.sideToMove() == Color::BLACK && words[i] == "btime"){
                            max_hard_time_ms = std::stoll(words[i+1]) / HARD_TM_RATIO;
                            max_soft_time_ms = std::stoll(words[i+1]) / SOFT_TM_RATIO;
                            // Immediately break up of the loop to not waste time
                            break;
                        }
                    }
                }
            }

            search_start_time = chrono::system_clock::now();
            search_root(board);
        }

        // Non-standard UCI command. Gets the engine to search at exactly
        // the specified depth -- ie. No iterative deepening. Commands
        // should look like search <depth>
        else if (words[0] == "search"){
            global_depth = 0;
            total_nodes = 0;
            max_hard_time_ms = 10000000000;
            max_soft_time_ms = 10000000000;
            int16_t depth = stoi(words[1]);
            int16_t score = alpha_beta(board, depth, DEFAULT_ALPHA, DEFAULT_BETA, 0);
            cout << "info score cp " << score << "\n";
            cout << "bestmove " << uci::moveToUci(root_best_move) << "\n"; 
        }

        // Non-standard UCI command, but very useful for debugging purposes.
        // Some engines use "d" to print the board as in "display" but it is
        // more verbose to just use "print"
        else if (words[0] == "print")
            print_board(board);

        // Non-standard UCI command for printing time management info
        else if (words[0] == "time"){
            cout << "info string soft bound " << max_soft_time_ms << "\n";
            cout << "info string hard bound " << max_hard_time_ms << "\n";
        }

        // Mostly for debugging purposes. This is a nonstandard UCI command
        // When "seval" is called, we return the static evaluation of the
        // current board position (relative to the current player)
        else if (words[0] == "seval")
            cout << evaluate(board) << "\n";

        // When the single match our tournament is over and the GUI doesn't
        // need our engine anymore it sends the "quit" command. Upon
        // receiving this command we end the uci loop and exit our program. 
        else if (words[0] == "quit")
            break;

    }

    return 0;
}

