# Weak Chess Engine

<p align="center">
  <img width="140.75" height="216.75" alt="image" src="https://github.com/user-attachments/assets/4b2c3769-cd6f-45d5-90df-6177eac07ebe" />
</p>

**Weak Chess Engine** is a chess engine written in C++ using [Disservin's Chess Library](https://github.com/Disservin/chess-library) . It speaks the [UCI protocol](https://en.wikipedia.org/wiki/Universal_Chess_Interface) and implements a alpha-beta fail-soft negamax framework for its search and tuned evaluation function.

> ⚠️ It's called *Weak* for a reason -- but it's built with care.

---

## Features

* Search
  * Fail-soft negamax framework
  * Iterative deepening
  * Aspiration window
  * Alpha beta pruning
  * Principal variation search
    * Triple-PVS
  * Quiescence search
    * Stand-Pat pruning
    * Delta pruning
    * SEE pruning
    * Late move pruning
  * Move Ordering
    * Transposition moves
    * MVV-LVA
    * SEE
    * Killer-move history
    * History gravity bonus and malus
      * Butterfly history
      * Continuation histories
        * 1-ply conthist (countermoves)
        * 2-ply conthist (follow-up moves)
  * Transposition table
    * Cutoffs
    * Ordering
  * Selectivity
    * Reverse futility pruning
    * Razoring
    * Null move pruning
    * Internal iterative reductions
    * Singular extensions
      * Multi-cut
    * History leaf pruning
    * Late move pruning
    * Futility pruning
    * SEE pruning
      * Captures
      * Quiets
    * Late move reductions
    * History reductions
    * Eval complexity reductions
    * Check extensions
  * Others
    * Static evaluation correction history
      * Pawn
      * Non-Pawn
      * Minor
      * Major
* Evaluation
  * Tuned evaluation
  * SWAR-Compressed evaluation
  * Tapered evaluation
  * Piece square tables
  * Mobility
  * Bishop pair
  * Passed pawns
  * King zone attacks
  * Doubled pawns
  * Pawn storm
  * Isolated pawns
  * Threats
  * Rook on semi-open file

---

## Playing Instructions
Weak does not come with its own GUI interface. To play against weak, you can use any suitable chess GUI interface such as Cutechess-GUI, Banksia, Arena, En-Croissant etc. You can also challenge Weak at [lichess](https://lichess.org/@/WeakChessEngine) although its rarely online. 

---

## Build Instructions

### Requirements

* A C++17-compatible compiler (preferably `g++`)

### Quick Compile

```bash
g++ -O3 -std=c++17 *.cpp -o weak
```

### Or
```bash
make
```

## Usage

Run from the command line or load it into a UCI-compatible GUI.

```bash
./weak
uci
isready
go
```

Or configure it in a GUI like **CuteChess** or **Arena**.

---

## Non-Standard UCI Commands
* `print` - Prints the board position
* `seval` - Prints the current static evaluation
* `search <depth>` - Searches to a specified depth and prints search info
* `time` - Prints the current time management info
* `see <move>` - Prints the SEE bolean for that move
* `obpasta` - Prints OpenBench SPSA Config

---

## UCI Options
* `Hash` - The transposition hash
* `Threads` - Number of threads to run on.
* `Move Overhead` - Number of ms to reduce from the time given due to communication overhead.

---

## Credits

### Engines I took inspiration from (in no particular order)
* [Ethereal](https://github.com/AndyGrant/Ethereal) - Lots of gainers as well as well-documented and easy-to-yoink code. Best chess engine to use as reference imho,
* [Potential](https://github.com/ProgramciDusunur/Potential) - Easy to read code with various new techniques implemented. Corrhist is based on Potential's one. ProgramciDusunur also wrote some good patches for Weak.
* [Sirius](https://github.com/mcthouacbb/Sirius) - Strongest HCE engine out there excluding SF HCE. Lots of new ideas and yoinkable stuff. LMR Corrplexity and other search stuff was based on Sirius. Original values for some params are also taken from Sirius.
* [Hobbes](https://github.com/kelseyde/hobbes-chess-engine) - Occasional reference to see how things should be done. Also the MattBench community engine.
* [Stash](https://gitlab.com/mhouppin/stash-bot) - Used to do progtests and estimate my engine's elo rating.

### People who provided help in the development process
* Disservin - Used chess.hpp library, which also had really good documentation.
* ProgramciDusunur [Potential] - Gave advice on lots of stuff like tuning, search, and also submitted 3 huge gainers
* kelseyde [Calvin] - Advice on corrhist and NMP
* JonathanHallstrom/swedishchef [Pawnocchio] - Helped with general stuff as well as helping me set up OB correctly
* nocturn9x/Matt [Heimdall] - Setting up mattbench and allowing me to join
* Bobingstern [Tarnished] - Helped with A LOT of general stuff as well as debugging UBs and info regarding elo gains
* The whole of MattBench server, Stockfish server, Engine Programming server and Sebastian Lague server for getting me into chess programming.
---
