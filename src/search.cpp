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

// Fail-high count for lmr [ply]
// Since we reset failhaigh count of ply+1, and our max ply is 255,
// we must have 256 + 1 = 257 elements
int32_t fail_high_count[257]{};

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
    int32_t raw_eval = tt_hit ? entry.raw_eval : evaluate(board);

    // Correct static evaluation with our correction histories
    int32_t eval = corrhist_adjust_eval(board, raw_eval);

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
    // STC: 14.58 +- 10.13 
    if (eval + max(delta_pruning_pawn_bonus.current, move_best_case_value(board)) < alpha)
        return eval;

    // Get all legal moves for our moveloop in our search
    Movelist capture_moves{};
    movegen::legalmoves<movegen::MoveGenType::CAPTURE>(capture_moves, board);

    std::array<bool, 256> see_bools{};

    // Move ordering
    if (capture_moves.size() != 0) { 
        see_bools = sort_captures(board, capture_moves, tt_hit, entry.best_move);
    }

    // Qsearch pruning stuff
    int32_t moves_played = 0;

    Move current_best_move{};
    
    for (int idx = 0; idx < capture_moves.size(); idx++){

        // QSearch movecount pruning
        // STC: 25.78 +- 14.91
        if (!board.inCheck() && moves_played >= 2)
            break;

        Move current_move = capture_moves[idx];

        // QSEE pruning, if a move is obviously losing, don't search it
        // STC: 179.35 +/- 31.54
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
            if (score > alpha){
                alpha = score;

                // Alpha-Beta Pruning
                if (alpha >= beta)
                    break;
            }
        }
    }

    NodeType bound = best_score >= beta ? NodeType::LOWERBOUND : best_score > old_alpha ? NodeType::EXACT : NodeType::UPPERBOUND;
    uint16_t best_move_tt = bound == NodeType::UPPERBOUND ? entry.best_move : current_best_move.move();

    // Storing transpositions
    tt.store(zobrists_key, best_score, raw_eval, 0, bound, best_move_tt, tt_hit ? entry.tt_was_pv : false);

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

    // Depth <= 0 (because we allow depth to drop below 0) and 
    // perform a qsearch to stabilise evaluation and avoid
    // the horizon effect
    if (depth <= 0){
        return q_search(board, alpha, beta, ply);
    }

    // Reset fail-high count for next ply
    fail_high_count[ply + 1] = 0;

    // Get the TT Entry for current position
    TTEntry entry{};
    uint64_t zobrists_key = board.hash(); 
    bool tt_hit = tt.probe(zobrists_key, entry);
    bool tt_was_pv = tt_hit ? entry.tt_was_pv : pv_node;

    // Transposition Table cutoffs
    // Only cut with a greater or equal depth search
    if (!pv_node 
        && entry.depth >= depth 
        && tt_hit 
        && ((entry.type == NodeType::EXACT) || (entry.type == NodeType::LOWERBOUND && entry.score >= beta)  || (entry.type == NodeType::UPPERBOUND && entry.score <= alpha)) 
        && search_info.excluded == 0)
        return entry.score;

    // Static evaluation for pruning metrics
    int32_t raw_eval = tt_hit ? entry.raw_eval : evaluate(board);

    // Correct static evaluation with our correction histories
    // STC: 20.87 +- 9.48 (pawn)
    // STC: 3.49 +- 2.77 (non pawn)
    // STC: 9.93 +- 6.18 (minor)
    // STC: 15.42 +- 7.84 (major)
    int32_t static_eval = corrhist_adjust_eval(board, raw_eval);

    // Improving heuristic (Whether we are at a better position than 2 plies before)
    // bool improving = static_eval > search_info.parent_parent_eval && search_info.parent_parent_eval != -100000;

    // Reverse futility pruning / Static Null Move Pruning
    // If eval is well above beta, we assume that it will hold
    // above beta. We "predict" that a beta cutoff will happen
    // and return eval without searching moves
    // STC: 132.85 +/- 26.94
    if (!pv_node 
        && !tt_was_pv 
        && !in_check 
        && depth <= 8 
        && static_eval - reverse_futility_margin.current * depth >= beta 
        && search_info.excluded == 0){
        return (static_eval + beta) / 2;
    }

    // Razoring / Alpha pruning
    // For low depths, if the eval is so bad that a large margin scaled
    // with depth is still not able to raise alpha, we can be almost sure 
    // that it will not be able to in the next few depths
    // https://github.com/official-stockfish/Stockfish/blob/ce73441f2013e0b8fd3eb7a0c9fd391d52adde70/src/search.cpp#L833
    // STC: 6.34 +/- 4.77
    if (!pv_node 
        && !in_check 
        && depth <= 3 
        && static_eval + razoring_base.current + razoring_quad_mul.current * depth * depth <= alpha  
        && search_info.excluded == 0){
        return q_search(board, alpha, beta, ply + 1);
    }

    // Null move pruning. Basically, we can assume that making a move 
    // is always better than not making our move most of the time
    // except if it's in a zugzwang. Hence, if we skip out turn and
    // we still maintain beta, then we can prune early. Also do not
    // do NMP when tt suggests that it should fail immediately
    // STC: 83.93 +/- 19.67
    // STC: 5.83 +- 5.85 (dynamic)
    if (!pv_node 
        && !in_check 
        && static_eval >= beta 
        && depth >= 2 
        && (!tt_hit || !(entry.type == NodeType::UPPERBOUND) || entry.score >= beta) && (board.hasNonPawnMaterial(Color::WHITE) || board.hasNonPawnMaterial(Color::BLACK)) 
        && search_info.excluded == 0){
        
        board.makeNullMove();
        int32_t reduction = null_move_base.current + depth / null_move_divisor.current;
                                                                                        
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
    // STC: 10.02 +/- 6.36
    // STC: 9.64 +- 7.56 (cutnode)
    // STC: 7.27 +- 5.64 (patch)
    if ((pv_node || cut_node) 
        && !in_check 
        && depth >= 7 
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
    // 1st TT Move (STC: 354.04 +/- 42.86)
    // 2nd MVV-LVA (STC: 109.50 +/- 25.18 (Note this is when mvv-lva was buggy)) + SEE (STC: 24.53 +- 14.42)
    // 3rd Killers Moves (quiets) (STC: 29.88 +/- 10.55) and (STC: 10.77 +/- 6.38) for two killers
    // 4th Histories (quiets) (STC: 41.16 +/- 13.63)
    //      - 1 ply conthist (countermoves) (STC: 31.45 +- 16.47)
    //      - 2 ply conthist (follow-up moves) (STC: 6.57 +- 5.04)
    sort_moves(board, all_moves, tt_hit, entry.best_move, ply, search_info);

    for (int idx = 0; idx < all_moves.size(); idx++){

        int32_t new_depth = depth;
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
            // STC: 24.29 +- 10.37
            if (depth <= 4 
                && !in_check 
                && move_history < depth * depth * -quiet_history_pruning_quad.current) 
                break;

            // Late Move Pruning
            // STC: 33.58 +- 16.99
            if (move_count >= late_move_pruning_base.current + late_move_pruning_quad.current * depth * depth) 
                continue;

            // Futility Pruning
            // STC: 14.92 +- 10.00
            if (depth <= 4 
                && !pv_node 
                && !in_check 
                && (static_eval + futility_eval_base.current) + futility_depth_mul.current * depth <= alpha) 
                continue;
        }

        // Singular extensions
        // https://github.com/AndyGrant/Ethereal/blob/0e47e9b67f345c75eb965d9fb3e2493b6a11d09a/src/search.c#L1022
        // STC: 16.12 +- 10.90 (singular extensions)
        // STC: 6.67 +- 5.34 (multi-cut)
        // STC: 5.73 +- 4.23 (negative-extensions)
        // STC: 8.62 +- 5.57 (double extent) 
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
            
                // Duble extensions
                if (!pv_node && score < singular_beta - 16)
                    extension = 2;
            }

            // Multi-cut pruning
            else if (singular_beta >= beta)
                return singular_beta;
            
            // Negative extensions
            // Potential for multi-cut
            else if (entry.score >= beta)
                extension = -3;

            // Cutnode negative extensions
            else if (entry.score <= alpha && cut_node)
                extension = -1;
        }

        // Static Exchange Evaluation Pruning
        // STC: 43.06 +/- 14.89
        int32_t see_margin = !is_noisy_move ? depth * see_quiet_margin.current : depth * see_noisy_margin.current;
        if (!pv_node 
            && !see(board, current_move, see_margin) 
            && best_score > -POSITIVE_WIN_SCORE)
            continue;

        // Quiet late moves reduction - we have to trust that our
        // move ordering is good enough most of the time to order
        // best moves at the start
        if (!is_noisy_move && depth >= 3){

            // Basic lmr "loglog" formula
            // STC: 35.76 +/- 12.65
            reduction += (int32_t)(((double)late_move_reduction_base.current / 100) + (((double)late_move_reduction_multiplier.current * log(depth) * log(move_count)) / 100));

            // History reductions - reduce more with bad histories and reduce
            // less with good histories
            // STC: 14.67 +- 7.79 (1st implementation)
            // STC: 16.36 +- 8.30 (2nd implementation)
            // Total STC Elo: ~31 elo
            reduction -= move_history / history_reduction_div.current;

            // LMR corrplexity - reduce less if we are in a complex 
            // position, determined by the difference between corrected eval
            // and raw evaluation
            // STC: 9.63 +- 6.01
            reduction -= abs(raw_eval - static_eval) > late_move_reduction_corrplexity.current;

            // Reduce more when in check
            // Ultra scaler somehow
            // STC: 3.86 +- 3.10
            // LTC: 16.23 +- 8.89
            reduction += in_check;

            // LMR Futility
            // Similar concept to futility pruning but we can be more aggressive
            // STC: 8.66 +- 5.65
            reduction += static_eval + lmr_futility_base.current + lmr_futility_multiplier.current * depth <= alpha; 

            // Fail-High LMR
            // Reduce more if this branch is known to fail high
            // STC: 5.41 +- 4.10
            reduction += !is_root && fail_high_count[ply + 1] > 2;

            // Reduce less in ttpv nodes
            // STC: 5.18 +- 3.94
            reduction -= tt_was_pv;

            // Reduce more in cut nodes
            // STC: 6.47 +- 4.60
            reduction += cut_node;

            // Reduce less for killer moves
            // STC: 5.45 +- 4.10
            reduction -= (killers[0][ply] == current_move) || (killers[1][ply] == current_move);
        }

        // Capture late move reductions - since the move is a capture
        // we have to be more careful as to how much we reduce as capture 
        // moves tend to be more noisy
        if (is_noisy_move && depth >= 3 && (!tt_was_pv || cut_node)){

            // Basic lmr "loglog" formula
            // STC: 17.07 +- 8.42
            reduction += (int32_t)(((double)capt_lmr_base.current / 100) + (((double)capt_lmr_multiplier.current * log(depth) * log(move_count)) / 100));

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
        // STC: 19.66 +/- 8.51,
        if (extension == 0 && board.inCheck())
            extension++;

        if (!is_noisy_move) 
            quiets_searched[quiets_searched_idx++] = current_move;

        // To update continuation history
        SearchInfo info{};
        info.parent_parent_move_piece = parent_move_piece;
        info.parent_parent_move_square = parent_move_square;
        info.parent_move_piece = move_piece;
        info.parent_move_square = to;

        // Adjust new depth
        new_depth = depth + extension - 1;

        // Principle Variation Search
        // STC: 54.25 +/- 15.03
        // STC: 27.39 +/- 11.81 (Triple PVS)
        if (move_count == 1)
            score = -alpha_beta(board, new_depth, -beta, -alpha, ply + 1, false, info);
        else {
            // LMR Moves
            if (reduction > 0){
                score = -alpha_beta(board, new_depth - reduction, -alpha - 1, -alpha, ply + 1, true, info);

                // Triple PVS research if reduced score beats alpha
                // We have dynamic conditions to change the depth of 
                // our research based on how far our score is away
                // from the bestscore. The conditions and initial
                // untuned values are taken from Sirius.
                // STC: 4.50 +- 3.49
                bool do_deeper = score > best_score + 37 + 139 * new_depth / 64;
                bool do_shallower = score < best_score + 8;

                if (score > alpha){ 
                    new_depth += do_deeper - do_shallower;                                        
                    score = -alpha_beta(board, new_depth, -alpha - 1, -alpha, ply + 1, !cut_node, info);
                }
            }
            else {
                score = -alpha_beta(board, new_depth, -alpha - 1, -alpha, ply + 1, !cut_node, info);
            }

            // Research
            if (score > alpha && score < beta) {
                score = -alpha_beta(board, new_depth, -beta, -alpha, ply + 1, false, info);
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
                if (alpha >= beta){

                    // Update fail-high count
                    fail_high_count[ply]++;

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
                        int32_t bonus = min(history_bonus_mul_quad.current * depth * depth + history_bonus_mul_linear.current * depth + history_bonus_base.current, 2048);
                        quiet_history[turn][from][to] += bonus - quiet_history[turn][from][to] * abs(bonus) / MAX_HISTORY;

                        // Continuation History Update
                        // 1-ply (Countermoves)
                        if (parent_move_piece != -1 && parent_move_square != -1){
                            one_ply_conthist[parent_move_piece][parent_move_square][move_piece][to] += bonus - one_ply_conthist[parent_move_piece][parent_move_square][move_piece][to] * abs(bonus) / MAX_HISTORY;
                        }
                        
                        // 2-ply (Follow-up moves)
                        if (parent_parent_move_piece != -1 && parent_parent_move_square != -1){
                            two_ply_conthist[parent_parent_move_piece][parent_parent_move_square][move_piece][to] += bonus - two_ply_conthist[parent_parent_move_piece][parent_parent_move_square][move_piece][to] * abs(bonus) / MAX_HISTORY;
                        }

                        // All History Malus
                        int32_t malus = min(history_malus_mul_quad.current * depth * depth + history_malus_mul_linear.current * depth + history_malus_base.current, 1024);
                        for (int32_t i = 0; i < quiets_searched_idx - 1; i++){
                            Move quiet = quiets_searched[i];
                            from = quiet.from().index();
                            to = quiet.to().index();
                            move_piece = static_cast<int32_t>(board.at(quiet.from()).internal());

                            // Quiet History Malus
                            // STC: 35.24 +/- 13.39
                            if (!board.isCapture(current_best_move)){
                                quiet_history[turn][from][to] = clamp(quiet_history[turn][from][to] - malus, -MAX_HISTORY, MAX_HISTORY);
                            }

                            // Conthist Malus
                            // 1-ply (Countermoves)
                            if (parent_move_piece != -1 && parent_move_square != -1){
                                one_ply_conthist[parent_move_piece][parent_move_square][move_piece][to] = clamp(one_ply_conthist[parent_move_piece][parent_move_square][move_piece][to] - malus, -MAX_HISTORY, MAX_HISTORY);
                            }

                            // 2-ply (Follow-up moves)
                            if (parent_parent_move_piece != -1 && parent_parent_move_square != -1){
                                two_ply_conthist[parent_parent_move_piece][parent_parent_move_square][move_piece][to]  = clamp(two_ply_conthist[parent_parent_move_piece][parent_parent_move_square][move_piece][to] - malus, -MAX_HISTORY, MAX_HISTORY);
                            }
                        }
                    }

                    break;
                }
            }
        }
    }

    if (move_count == 0 && search_info.excluded != 0)
        return alpha;

    // Don't store TT in singular searches
    if (search_info.excluded == 0){
        NodeType bound = best_score >= beta ? NodeType::LOWERBOUND : alpha > old_alpha ? NodeType::EXACT : NodeType::UPPERBOUND;
        uint16_t best_move_tt = bound == NodeType::UPPERBOUND ? entry.best_move : current_best_move.move();

        // Update correction histories
        if (!in_check && !board.isCapture(current_best_move) 
            && !(bound == NodeType::LOWERBOUND && best_score <= static_eval) 
            && !(bound == NodeType::UPPERBOUND && best_score >= static_eval)) {
            
            int32_t corrhist_bonus = clamp(best_score - static_eval, -1024, 1024);
            update_correction_history(board, depth, corrhist_bonus);
        }

        // Storing transpositions
        tt.store(zobrists_key, best_score, raw_eval, depth, bound, best_move_tt, tt_was_pv);
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
        // STC: 40.83 +/- 13.86
        // STC: 7.34 +/- 5.30 (bugfix 1)
        // STC:  12.94 +- 7.19 (bugfix 2)
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

            if (global_depth >= 4){
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
                    cout << "info depth " << global_depth << " seldepth " << seldpeth << " time " << elapsed_time << " score cp " << alpha << " upperbound nodes " << total_nodes << " nps " <<   (1000 * total_nodes) / (elapsed_time + 1) << " hashfull " << tt.hashfull() << " pv " << uci::moveToUci(root_best_move);
                    
                    Board new_board = Board(board.getFen());
                    new_board.makeMove(root_best_move);
                    print_tt_pv(new_board, max(global_depth - 1, 0));
                    cout << endl;

                    beta = (alpha + beta) / 2;
                    alpha = max(-POSITIVE_INFINITY, alpha - delta);
                }

                // Lowerbound
                else if (new_score >= beta){
                    cout << "info depth " << global_depth << " seldepth " << seldpeth << " time " << elapsed_time << " score cp " << beta << " lowerbound nodes " << total_nodes << " nps " <<   (1000 * total_nodes) / (elapsed_time + 1) << " hashfull " << tt.hashfull() << " pv " << uci::moveToUci(root_best_move);
                    
                    Board new_board = Board(board.getFen());
                    new_board.makeMove(root_best_move);
                    print_tt_pv(new_board, max(global_depth - 1, 0));
                    cout << endl;

                    beta = min(POSITIVE_INFINITY, beta + delta);
                }

                // Score falls within window (exact)
                else {
                    cout << "info depth " << global_depth << " seldepth " << seldpeth << " time " << elapsed_time << " score cp " << new_score << " nodes " << total_nodes << " nps " <<   (1000 * total_nodes) / (elapsed_time + 1) << " hashfull " << tt.hashfull() << " pv " << uci::moveToUci(root_best_move);
                    
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