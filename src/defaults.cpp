#include "defaults.hpp"

// Basics
SearchParam tt_size("Hash", 64, 1, 16384, 1);
SearchParam threads("Threads", 1, 1, 1, 1);
SearchParam move_overhead("Move Overhead", 0, 0, 10000, 1);

// SPSA (https://chess.n9x.co/tune/3573/)
/*
ReverseFutilityMargin, 49
NullMoveBase, 4
NullMoveDivisor, 4
LateMoveReductionBase, 68
LateMoveReductionMultiplier, 50
LateMoveReductionCorrplexity, 87
LateMovePruningBase, 3
LateMovePruningQuad, 3
FutilityEvalBase, 104
FutilityDepthMul, 101
AspirationWindowDelta, 9
AspirationWideningFactor, 3
SeeNoisyMargin, -98
SeeQuietMargin, -55
QuietHistoryPruningQuad, 2004
HistoryBonusBase, 94
HistoryBonusMulLinear, 219
HistoryBonusMulQuad, 417
HistoryMalusBase, 1
HistoryMalusMulLinear, 260
HistoryMalusMulQuad, 145
HistoryReductionDepthMul, 992
RazoringBase, 616
RazoringQuadMul, 308
SEEPawn, 105
SEEKnight, 340
SEEBishop, 312
SEERook, 502
SEEQueen, 928
DeltaPruningPawnBonus, 134
Tempo, 4
SoftTMRatio, 29
HardTMRatio, 1
NodeTMBase, 211
NodeTMMul, 113
*/

SearchParam reverse_futility_margin("ReverseFutilityMargin", 49, 30, 100, 10);

SearchParam null_move_base("NullMoveBase", 4, 1, 6, 1);
SearchParam null_move_divisor("NullMoveDivisor", 4, 1, 8, 1);

SearchParam late_move_reduction_base("LateMoveReductionBase", 68, 30, 120, 15);
SearchParam late_move_reduction_multiplier("LateMoveReductionMultiplier", 50, 10, 70, 7);
SearchParam late_move_reduction_corrplexity("LateMoveReductionCorrplexity", 87, 50, 150, 7);

SearchParam late_move_pruning_base("LateMovePruningBase", 3, 1, 10, 1);
SearchParam late_move_pruning_quad("LateMovePruningQuad", 3, 1, 10, 1);

SearchParam futility_eval_base("FutilityEvalBase", 104, 50, 200, 15);
SearchParam futility_depth_mul("FutilityDepthMul", 101, 50, 200, 6);

SearchParam aspiration_window_delta("AspirationWindowDelta", 9, 5, 30, 2);
SearchParam aspiration_widening_factor("AspirationWideningFactor", 50, 1, 200, 10);

SearchParam see_noisy_margin("SeeNoisyMargin", -98, -120, -30, 6);
SearchParam see_quiet_margin("SeeQuietMargin", -55, -120, -30, 6);

SearchParam quiet_history_pruning_quad("QuietHistoryPruningQuad", 2004, 786, 8192, 64);

SearchParam history_bonus_base("HistoryBonusBase", 94, -384, 768, 64);
SearchParam history_bonus_mul_linear("HistoryBonusMulLinear", 219, 64, 384, 32);
SearchParam history_bonus_mul_quad("HistoryBonusMulQuad", 417, 1, 1536, 64);

SearchParam history_malus_base("HistoryMalusBase", 1, -384, 768, 64);
SearchParam history_malus_mul_linear("HistoryMalusMulLinear", 260, 64, 384, 32);
SearchParam history_malus_mul_quad("HistoryMalusMulQuad", 145, 1, 1536, 64);

SearchParam history_reduction_div("HistoryReductionDiv", 8192, 1024, 16384, 64);

SearchParam razoring_base("RazoringBase", 616, -384, 768, 64);
SearchParam razoring_quad_mul("RazoringQuadMul", 308, 1, 1536, 64);

SearchParam see_pawn("SEEPawn", 105, 70, 130, 10);
SearchParam see_knight("SEEKnight", 340, 250, 450, 15);
SearchParam see_bishop("SEEBishop", 312, 250, 450, 15);
SearchParam see_rook("SEERook", 502, 400, 650, 15);
SearchParam see_queen("SEEQueen", 928, 800, 1200, 15);

SearchParam delta_pruning_pawn_bonus("DeltaPruningPawnBonus", 134, 80, 200, 20);

SearchParam tempo("Tempo", 4, 0, 20, 3);

SearchParam soft_tm_ratio("SoftTMRatio", 29, 5, 50, 8);
SearchParam hard_tm_ratio("HardTMRatio", 1, 1, 20, 4);
SearchParam node_tm_base("NodeTMBase", 211, 50, 300, 20);
SearchParam node_tm_mul("NodeTMMul", 113, 50, 300, 20);
