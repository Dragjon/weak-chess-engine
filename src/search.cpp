#include <stdint.h>
#include <iostream>
#include <chrono> 
#include <algorithm>
#include <cmath>
#include <vector>

#include "chess.hpp"
#include "timeman.hpp"
#include "search.hpp"
#include "eval.hpp"
#include "transposition.hpp"
#include "ordering.hpp"
#include "see.hpp"
#include "defaults.hpp"
#include "history.hpp"
#include "moves.hpp"

using namespace chess;
using namespace std;

// Summoning multithread demons with global vars!!
// Storing the final best move for every complete search
chess::Move root_best_move{};
chess::Move previous_best_move{};

int64_t best_move_nodes = 0;

// BM-stability
int32_t bm_stability = 0;

// Score stability
int32_t score_stability = 0;
int32_t avg_prev_score = 0;
int32_t root_best_score = 0;

int64_t total_nodes_per_search = 0;

int32_t global_depth = 0;
int64_t total_nodes = 0;

// Highest searched depth
int32_t seldpeth = 0;


// Quiescence search. When we are in a noisy position (there are captures), we try to "quiet" the position by
// going down capture trees using negamax and return the eval when we re in a quiet position
int32_t q_search(Board &board, int32_t alpha, int32_t beta, int32_t ply){
    // Increment node count
    total_nodes++;
    total_nodes_per_search++;

    // Handle time management
    // Here is also where our hard-bound time mnagement is. When the search time 
    // exceeds our maximum hard bound time limit
    if (global_depth > 1 && hard_bound_time_exceeded())
        throw SearchAbort();

    // Update highest searched depth
    if (ply > seldpeth)
        seldpeth = ply;

    // Draw detections
    if ((board.isHalfMoveDraw() || board.isInsufficientMaterial() || board.isRepetition(1)))
        return 0;

    // Get the TT Entry for current position
    TTEntry entry{};
    uint64_t zobrists_key = board.hash(); 
    bool tt_hit = tt.probe(zobrists_key, entry);

    // Transposition Table cutoffs
    if (tt_hit && ((entry.type == NodeType::EXACT) || (entry.type == NodeType::LOWERBOUND && entry.score >= beta)  || (entry.type == NodeType::UPPERBOUND && entry.score <= alpha)))
        return entry.score;
        
    // For TT updating later to determine bound
    int32_t old_alpha = alpha;

    // Eval pruning - If a static evaluation of the board will
    // exceed beta, then we can stop the search here. Also, if the static
    // eval exceeds alpha, we can set alpha to our new eval (comment from Ethereal)
    int32_t eval = evaluate(board);

    // Correct static evaluation with our correction histories
    eval = corrhist_adjust_eval(board, eval);

    int32_t best_score = eval;
    if (best_score >= beta) return best_score;
    if (best_score > alpha) alpha = best_score;

    // Max ply cutoff
    if (ply >= MAX_SEARCH_PLY)
        return alpha;

    // Delta Pruning - Even when best possible capture is made + promotion combination
    // is still not enough to cover the distance between alpha and eval, playing a move
    // is futile. Minor boost for pawn captures idea from Ethereal: 
    // https://github.com/AndyGrant/Ethereal/blob/0e47e9b67f345c75eb965d9fb3e2493b6a11d09a/src/search.c#L872
    if (eval + max(142, move_best_case_value(board)) < alpha)
        return eval;

    // Get all legal moves for our moveloop in our search
    Movelist capture_moves{};
    movegen::legalmoves<movegen::MoveGenType::CAPTURE>(capture_moves, board);

    vector<bool> see_bools{};
    // Move ordering
    if (capture_moves.size() != 0) { 
        see_bools = sort_captures(board, capture_moves, tt_hit, entry.best_move);
    }

    // Qsearch pruning stuff
    int32_t moves_played = 0;

    Move current_best_move{};
    
    for (int idx = 0; idx < capture_moves.size(); idx++){

        // QSearch movecount pruning
        if (!board.inCheck() && moves_played >= 2)
            break;

        Move current_move = capture_moves[idx];

        // QSEE pruning, if a move is obviously losing, don't search it
        if (!see_bools[idx]) 
            continue;

        // Basic make and undo functionality. Copy-make should be faster but that
        // debugging is for later
        board.makeMove(current_move);
        moves_played++;
        int32_t score = -q_search(board, -beta, -alpha, ply + 1);
        board.unmakeMove(current_move);

        // Updating best_score and alpha beta pruning
        if (score > best_score){
            best_score = score;
            current_best_move = current_move;

            // Update alpha
            if (score > alpha)
                alpha = score;

            // Alpha-Beta Pruning
            if (score >= beta)
                break;
        }
    }

    NodeType bound = best_score >= beta ? NodeType::LOWERBOUND : best_score > old_alpha ? NodeType::EXACT : NodeType::UPPERBOUND;
    uint16_t best_move_tt = bound == NodeType::UPPERBOUND ? 0 : current_best_move.move();

    // Storing transpositions
    tt.store(zobrists_key, clamp(best_score, -40000, 40000), 0, bound, best_move_tt, tt_hit ? entry.tt_was_pv : false);

    return best_score;
}

// Search Function
// We are basically using a fail soft "negamax" search, see here for more info: https://minuskelvin.net/chesswiki/content/minimax.html#negamax
// Negamax is basically a simplification of the famed minimax algorithm. Basically, it works by negating the score in the next
// ply. This works because a position which is a win for white is a loss for black and vice versa. Most "strong" chess engines use
// negamax instead of minimax because it makes the code much tidier. Not sure about how much is gains though. The "fail soft" basically
// means we return max_value instead of alpha. This gives us more information to do puning etc etc.
int32_t alpha_beta(Board &board, int32_t depth, int32_t alpha, int32_t beta, int32_t ply, bool cut_node, SearchInfo search_info){

    // Search variables
    // max_score for fail-soft negamax
    int32_t best_score = -POSITIVE_INFINITY;
    bool is_root = ply == 0;
    bool in_check = board.inCheck();

    // Important for cutoffs
    // I'm aware this is not the best way to do it but that's for later
    bool pv_node = beta - alpha > 1;

    int32_t parent_move_piece = search_info.parent_move_piece;
    int32_t parent_move_square = search_info.parent_move_square;
    int32_t parent_parent_move_piece = search_info.parent_parent_move_piece;
    int32_t parent_parent_move_square = search_info.parent_parent_move_square;

    // For updating Transposition table later
    int32_t old_alpha = alpha;  

    // Increment node count
    total_nodes++;
    total_nodes_per_search++;

    // Handle time management
    // Here is where our hard-bound time mnagement is. When the search time 
    // exceeds our maximum hard bound time limit
    if (global_depth > 1 && hard_bound_time_exceeded())
        throw SearchAbort();

     // Update highest searched depth
    if (ply > seldpeth)
        seldpeth = ply;

    // Draw detections
    // Ensure all drawn positions have a score of 0. This is important so
    // our engine will not mistake a drawn position with a winning one
    // Say we are up a bishop in the endgame. Although static eval may tell
    // us that we are +bishop_value, the actual eval should be 0. Some 
    // engines have a "contempt" variable in draw detections to make engines
    // avoid draws more or like drawn positions. This surely weakens the
    // engine when playing against another at the same level. But it is
    // irrelevant in our case.
    if (!is_root && (board.isHalfMoveDraw() || board.isInsufficientMaterial() || board.isRepetition(1)))
        return 0;

    // Get all legal moves for our moveloop in our search
    Movelist all_moves{};
    movegen::legalmoves(all_moves, board);

    // Checkmate detection
    // When we are in checkmate during our turn, we lost the game, therefore we 
    // should return a large negative value
    if (!is_root && all_moves.size() == 0){
        if (in_check)
            return -POSITIVE_MATE_SCORE + ply;

        // Stalemate jumpscare!
        else 
            return 0;
    }

    // Max ply cutoff to avoid ubs with our arrays
    if (ply >= MAX_SEARCH_PLY){
        return evaluate(board);
    }

    // Depth <= 0 (because we allow depth to drop below 0) - we end our search and return eval (haven't started qs yet)
    if (depth <= 0){
        return q_search(board, alpha, beta, ply);
    }

    // Get the TT Entry for current position
    TTEntry entry{};
    uint64_t zobrists_key = board.hash(); 
    bool tt_hit = tt.probe(zobrists_key, entry);
    bool tt_was_pv = tt_hit ? entry.tt_was_pv : pv_node;

    // Transposition Table cutoffs
    // Only cut with a greater or equal depth search
    if (!pv_node 
        && entry.depth >= depth 
        && !is_root 
        && tt_hit 
        && ((entry.type == NodeType::EXACT) || (entry.type == NodeType::LOWERBOUND && entry.score >= beta)  || (entry.type == NodeType::UPPERBOUND && entry.score <= alpha)) 
        && search_info.excluded == 0)
        return entry.score;

    // Static evaluation for pruning metrics
    int32_t raw_eval = evaluate(board);

    // Correct static evaluation with our correction histories
    int32_t static_eval = corrhist_adjust_eval(board, raw_eval);

    // Improving heuristic (Whether we are at a better position than 2 plies before)
    // bool improving = static_eval > search_info.parent_parent_eval && search_info.parent_parent_eval != -100000;

    // Reverse futility pruning / Static Null Move Pruning
    // If eval is well above beta, we assume that it will hold
    // above beta. We "predict" that a beta cutoff will happen
    // and return eval without searching moves
    if (!pv_node 
        && !tt_was_pv 
        && !in_check 
        && depth <= reverse_futility_depth.current 
        && static_eval - reverse_futility_margin.current * depth >= beta 
        && search_info.excluded == 0){
        return (static_eval + beta) / 2;
    }

    // Razoring / Alpha pruning
    // For low depths, if the eval is so bad that a large margin scaled
    // with depth is still not able to raise alpha, we can be almost sure 
    // that it will not be able to in the next few depths
    // https://github.com/official-stockfish/Stockfish/blob/ce73441f2013e0b8fd3eb7a0c9fd391d52adde70/src/search.cpp#L833
    if (!pv_node 
        && !in_check 
        && depth <= razoring_max_depth.current 
        && static_eval + razoring_base.current + razoring_linear_mul.current * depth + razoring_quad_mul.current * depth * depth <= alpha  
        && search_info.excluded == 0){
        return q_search(board, alpha, beta, ply + 1);
    }

    // Null move pruning. Basically, we can assume that making a move 
    // is always better than not making our move most of the time
    // except if it's in a zugzwang. Hence, if we skip out turn and
    // we still maintain beta, then we can prune early. Also do not
    // do NMP when tt suggests that it should fail immediately
    if (!pv_node 
        && !in_check 
        && static_eval >= beta 
        && depth >= null_move_depth.current 
        && (!tt_hit || !(entry.type == NodeType::UPPERBOUND) || entry.score >= beta) && (board.hasNonPawnMaterial(Color::WHITE) || board.hasNonPawnMaterial(Color::BLACK)) 
        && search_info.excluded == 0){
        
        board.makeNullMove();
        int32_t reduction = 3 + depth / 3;
                                                                                        
        // Search has no parents :(
        SearchInfo info{};                                                                   
        info.parent_parent_move_piece = parent_move_piece;
        info.parent_parent_move_square = parent_move_square;                                // Child of a cut node is a all-node and vice versa
        int32_t null_score = -alpha_beta(board, depth - reduction, -beta, -beta+1, ply + 1, !cut_node, info);
        board.unmakeNullMove();

        if (null_score >= beta)
            // Do not return false mates in null move pruning (patch)
            return abs(null_score) >= POSITIVE_WIN_SCORE ? beta : null_score;
    }

    // Internal iterative reduction. Artifically lower the depth on pv nodes / cutnodes
    // that are high enough up in the search tree that we would expect to find a transposition
    // to use later. (Comment from Ethereal)
    if ((pv_node || cut_node) 
        && !in_check 
        && depth >= internal_iterative_reduction_depth.current 
        && (!tt_hit || (entry.best_move != 0 && entry.depth <= depth - 5)) 
        && search_info.excluded == 0)
        depth--;

    // Main move loop
    // For loop is faster than foreach :)
    Move current_best_move{};
    int32_t move_count = 0;

    // Store quiets searched for history malus
    Move quiets_searched[1024]{};
    int32_t quiets_searched_idx = 0;

    // Clear killers of next ply
    killers[0][ply+1] = Move{}; 
    killers[1][ply+1] = Move{}; 

    // Move orderings
    // 1st TT Move
    // 2nd MVV-LVA (captures)
    // 3rd Killers Moves (quiets)
    // 4th Histories (quiets)
    //      - 1 ply conthist (countermoves)
    //      - 2 ply conthist (follow-up moves)
    sort_moves(board, all_moves, tt_hit, entry.best_move, ply, search_info);

    for (int idx = 0; idx < all_moves.size(); idx++){

        int32_t reduction = 0;
        int32_t extension = 0;
        int64_t nodes_b4 = total_nodes;

        Move current_move = all_moves[idx];

        // Skip excluded singular moves
        if (current_move.move() == search_info.excluded)
            continue;

        move_count++;

        bool is_noisy_move = board.isCapture(current_move);

        int32_t move_history = !is_noisy_move ? quiet_history[board.sideToMove() == chess::Color::WHITE][current_move.from().index()][current_move.to().index()] : 0;

        // Quiet Move Prunings
        if (!is_root && !is_noisy_move && best_score > -POSITIVE_WIN_SCORE) {
            // Quiet History Pruning
            if (depth <= 4 
                && !in_check 
                && move_history < depth * depth * -2048) 
                break;

            // Late Move Pruning
            if (move_count >= 4 + 3 * depth * depth) 
                continue;

            // Futility Pruning
            if (depth < 5 
                && !pv_node 
                && !in_check 
                && (static_eval + 100) + 100 * depth <= alpha) 
                continue;
        }

        // Singular extensions
        // https://github.com/AndyGrant/Ethereal/blob/0e47e9b67f345c75eb965d9fb3e2493b6a11d09a/src/search.c#L1022
        bool do_singular_search =  !is_root &&  depth >= 6 
                                    &&  current_move.move() == entry.best_move 
                                    &&  entry.depth >= depth - 3 
                                    && (entry.type == NodeType::LOWERBOUND) 
                                    && search_info.excluded == 0;

        if (do_singular_search)
        {
            SearchInfo se_info = search_info;
            int32_t value = max(-POSITIVE_MATE_SCORE, entry.score - depth);
            int32_t singular_beta = value;
            int32_t singular_depth = (depth - 1) / 2;

            se_info.excluded = entry.best_move;
            int32_t score = alpha_beta(board, singular_depth, singular_beta - 1, singular_beta, ply, cut_node, se_info); 

            if (score < singular_beta){
                extension = 1;

                // Double extensions
                if (!pv_node && score < singular_beta - 16)
                    extension++;
            }

            // Multi-cut pruning
            else if (singular_beta >= beta)
                return singular_beta;
        }

        // Static Exchange Evaluation Pruning
        int32_t see_margin = !is_noisy_move ? depth * see_quiet_margin.current : depth * see_noisy_margin.current;
        if (!pv_node 
            && !see(board, current_move, see_margin) 
            && alpha < POSITIVE_WIN_SCORE)
            continue;

        // Quiet late moves reduction - we have to trust that our
        // move ordering is good enough most of the time to order
        // best moves at the start
        if (!is_noisy_move && depth >= late_move_reduction_depth.current){

            // Basic lmr "loglog" formula
            reduction += (int32_t)(((double)late_move_reduction_base.current / 100) + (((double)late_move_reduction_multiplier.current * log(depth) * log(move_count)) / 100));

            // Custom history reductions, reduce if a move's quiet 
            // history score is very low, scaled by depth
            reduction += move_history < -1024 * depth;

            // LMR corrplexity - reduce less if we are in a complex 
            // position, determined by the difference between corrected eval
            // and raw evaluation
            reduction -= abs(raw_eval - static_eval) > 89;
        }

        int32_t score = 0;
        bool turn = board.sideToMove() == chess::Color::WHITE;
        int32_t to = current_move.to().index();
        int32_t from = current_move.from().index();
        int32_t move_piece = static_cast<int32_t>(board.at(current_move.from()).internal());

        // Basic make and undo functionality. Copy-make should be faster but that
        // debugging is for later
        board.makeMove(current_move);

        // Check extension, we increase the depth of moves that give check
        // This helps mitigate the horizon effect where noisy nodes are 
        // mistakenly evaluated
        if (board.inCheck())
            extension++;

        if (!is_noisy_move) 
            quiets_searched[quiets_searched_idx++] = current_move;

        // To update continuation history
        SearchInfo info{};
        info.parent_parent_move_piece = parent_move_piece;
        info.parent_parent_move_square = parent_move_square;
        info.parent_move_piece = move_piece;
        info.parent_move_square = to;

        // Principle Variation Search
        if (move_count == 1)
            score = -alpha_beta(board, depth + extension - 1, -beta, -alpha, ply + 1, false, info);
        else {
            score = -alpha_beta(board, depth - reduction + extension - 1, -alpha - 1, -alpha, ply + 1, true, info);

            // Triple PVS
            if (reduction > 0 && score > alpha){                                          
                score = -alpha_beta(board, depth + extension - 1, -alpha - 1, -alpha, ply + 1, !cut_node, info);
            }

            // Research
            if (score > alpha && score < beta) {
                score = -alpha_beta(board, depth + extension - 1, -beta, -alpha, ply + 1, false, info);
            }
        }

        board.unmakeMove(current_move);

        // Updating best_score and alpha beta pruning
        if (score > best_score){
            best_score = score;
            current_best_move = current_move;

            if (is_root){
                root_best_move = current_move;

                // Node time management, we get total number of nodes spent searching on best move
                // and scale our tm based on it
                best_move_nodes = total_nodes - nodes_b4;
            }

            // Update alpha
            if (score > alpha){
                alpha = score;

                // Alpha-Beta Pruning
                if (score >= beta){

                    // Quiet move heuristics
                    if (!is_noisy_move){
                        // Killer move heuristic
                        // We have 2 killers per ply
                        // We don't duplicate killers
                        if (current_move != killers[0][ply]){
                            killers[1][ply] = killers[0][ply]; 
                            killers[0][ply] = current_move;
                        }

                        // History Heuristic + gravity
                        int32_t bonus = clamp(history_bonus_mul_quad.current * depth * depth + history_bonus_mul_linear.current * depth + history_bonus_base.current, -MAX_HISTORY, MAX_HISTORY);
                        quiet_history[turn][from][to] += bonus - quiet_history[turn][from][to] * abs(bonus) / MAX_HISTORY;

                        // Continuation History Update
                        // 1-ply (Countermoves)
                        if (parent_move_piece != -1 && parent_move_square != -1){
                            int32_t conthist_bonus = clamp(500 * depth * depth + 200 * depth + 150, -MAX_HISTORY, MAX_HISTORY);
                            one_ply_conthist[parent_move_piece][parent_move_square][move_piece][to] += conthist_bonus - one_ply_conthist[parent_move_piece][parent_move_square][move_piece][to] * abs(conthist_bonus) / MAX_HISTORY;
                        }
                        
                        // 2-ply (Follow-up moves)
                        if (parent_parent_move_piece != -1 && parent_parent_move_square != -1){
                            int32_t conthist_bonus = clamp(500 * depth * depth + 200 * depth + 150, -MAX_HISTORY, MAX_HISTORY);
                            two_ply_conthist[parent_parent_move_piece][parent_parent_move_square][move_piece][to] += conthist_bonus - two_ply_conthist[parent_parent_move_piece][parent_parent_move_square][move_piece][to] * abs(conthist_bonus) / MAX_HISTORY;
                        }

                        // All History Malus
                        for (int32_t i = 0; i < quiets_searched_idx; i++){
                            Move quiet = quiets_searched[i];
                            from = quiet.from().index();
                            to = quiet.to().index();
                            move_piece = static_cast<int32_t>(board.at(quiet.from()).internal());

                            // Quiet History Malus
                            if (move_count > 1 && !board.isCapture(current_best_move)){
                                quiet_history[turn][from][to] -= history_malus_mul_quad.current * depth * depth + history_malus_mul_linear.current * depth + history_bonus_base.current;
                            }

                            // Conthist Malus
                            // 1-ply (Countermoves)
                            if (parent_move_piece != -1 && parent_move_square != -1){
                                one_ply_conthist[parent_move_piece][parent_move_square][move_piece][to] -= 300 * depth * depth + 280 * depth + 50;
                            }

                            // 2-ply (Follow-up moves)
                            if (parent_parent_move_piece != -1 && parent_parent_move_square != -1){
                                two_ply_conthist[parent_parent_move_piece][parent_parent_move_square][move_piece][to]  -= 300 * depth * depth + 280 * depth + 50;
                            }
                        }
                    }

                    break;
                }
            }
        }
    }

    if (move_count == 0 && search_info.excluded != 0){
        return alpha;
    }

    // Don't store TT in singular searches
    if (search_info.excluded == 0){
        NodeType bound = best_score >= beta ? NodeType::LOWERBOUND : alpha > old_alpha ? NodeType::EXACT : NodeType::UPPERBOUND;
        uint16_t best_move_tt = bound == NodeType::UPPERBOUND ? 0 : current_best_move.move();

        // Update correction histories
        if (!in_check && !board.isCapture(current_best_move) 
            && !(bound == NodeType::LOWERBOUND && best_score <= static_eval) 
            && !(bound == NodeType::UPPERBOUND && best_score >= static_eval)) {
            
            int32_t corrhist_bonus = clamp(best_score - static_eval, -1024, 1024);
            update_correction_history(board, depth, corrhist_bonus);
        }

        // Storing transpositions
        tt.store(zobrists_key, clamp(best_score, -40000, 40000), depth, bound, best_move_tt, tt_was_pv);
    }

    return best_score;

}

// Prints the Transposition PV
void print_tt_pv(Board &board, int32_t depth){

    if (depth > 0){
        TTEntry entry{};
        uint64_t zobrists_key = board.hash(); 
        tt.probe(zobrists_key, entry);
        Movelist all_moves{};
        movegen::legalmoves(all_moves, board);

        bool is_legal = false;
        for (int32_t i = 0; i < all_moves.size(); i++){
            if (all_moves[i].move() == entry.best_move){
                cout << " " << uci::moveToUci(all_moves[i]);
                board.makeMove(all_moves[i]);
                is_legal = true;
                break;
            }
        }

        if (is_legal){
            print_tt_pv(board, depth - 1);
        }
    }

}


// Iterative deepening time management loop
// Uses soft bound time management
int32_t search_root(Board &board){
    try {
        // Aspiration window search, we predict that the score from previous searches will be
        // around the same as the next depth +/- some margin.
        int32_t score = 0;
        int32_t delta = aspiration_window_delta.current;
        int32_t alpha = DEFAULT_ALPHA;
        int32_t beta = DEFAULT_BETA;
        while ((global_depth == 0 || !soft_bound_time_exceeded()) && global_depth < MAX_SEARCH_DEPTH){

            previous_best_move = root_best_move;

            // Increment the global depth since global_depth starts from 0
            global_depth++;
            int32_t researches = 0;
            int32_t new_score = 0;

            if (global_depth >= aspiration_window_depth.current){
                alpha = max(-POSITIVE_INFINITY, score - delta);
                beta = min(POSITIVE_INFINITY, score + delta);
            }

            while (true){

                total_nodes_per_search = 0ll;
                SearchInfo info{};
                new_score = alpha_beta(board, global_depth, alpha, beta, 0, false, info);
                int64_t elapsed_time = elapsed_ms();

                // Upperbound
                if (new_score <= alpha){
                    cout << "info depth " << global_depth << " seldepth " << seldpeth << " time " << elapsed_time << " score cp " << alpha << " upperbound nodes " << total_nodes << " nps " <<   (1000 * total_nodes) / (elapsed_time + 1) << " pv " << uci::moveToUci(root_best_move);
                    
                    Board new_board = Board(board.getFen());
                    new_board.makeMove(root_best_move);
                    print_tt_pv(new_board, max(global_depth - 1, 0));
                    cout << endl;

                    beta = (alpha + beta) / 2;
                    alpha = max(-POSITIVE_INFINITY, new_score - delta);
                }

                // Lowerbound
                else if (new_score >= beta){
                    cout << "info depth " << global_depth << " seldepth " << seldpeth << " time " << elapsed_time << " score cp " << beta << " lowerbound nodes " << total_nodes << " nps " <<   (1000 * total_nodes) / (elapsed_time + 1) << " pv " << uci::moveToUci(root_best_move);
                    
                    Board new_board = Board(board.getFen());
                    new_board.makeMove(root_best_move);
                    print_tt_pv(new_board, max(global_depth - 1, 0));
                    cout << endl;

                    beta = min(POSITIVE_INFINITY, new_score + delta);
                }

                // Score falls within window (exact)
                else {
                    cout << "info depth " << global_depth << " seldepth " << seldpeth << " time " << elapsed_time << " score cp " << new_score << " nodes " << total_nodes << " nps " <<   (1000 * total_nodes) / (elapsed_time + 1) << " pv " << uci::moveToUci(root_best_move);
                    
                    Board new_board = Board(board.getFen());
                    new_board.makeMove(root_best_move);
                    print_tt_pv(new_board, max(global_depth - 1, 0));
                    cout << endl;

                    break;
                }

                // If we exceed our time management, we stop widening 
                if (soft_bound_time_exceeded())
                    break;
                    
                else delta += delta * aspiration_widening_factor.current / 100;
            }

            score = new_score;
            root_best_score = score;

            // Score stability time management
            avg_prev_score = (avg_prev_score + root_best_score) / 2;
            
        }
    }

    // Hard-bound time management catch
    catch (const SearchAbort& e) { 
        
    }

    cout << "bestmove " << uci::moveToUci(root_best_move) << endl;

    return 0;
}