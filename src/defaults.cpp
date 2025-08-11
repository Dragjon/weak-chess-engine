#include "defaults.hpp"

// Basics
SearchParam tt_size("Hash", 64, 1, 16384, 1);
SearchParam threads("Threads", 1, 1, 1, 1);
SearchParam move_overhead("MoveOverhead", 0, 0, 10000, 1);

// SPSA (https://kelseyde.pythonanywhere.com/tune/969/)
/*
ReverseFutilityMargin, 46
NullMoveBase, 4
NullMoveDivisor, 3
LateMoveReductionBase, 89
LateMoveReductionMultiplier, 48
LateMoveReductionCorrplexity, 98
LateMovePruningBase, 4
LateMovePruningQuad, 1
FutilityEvalBase, 97
FutilityDepthMul, 90
AspirationWindowDelta, 11
AspirationWideningFactor, 18
SeeNoisyMargin, -101
SeeQuietMargin, -45
QuietHistoryPruningQuad, 2028
HistoryBonusBase, -51
HistoryBonusMulLinear, 205
HistoryBonusMulQuad, 411
HistoryMalusBase, 56
HistoryMalusMulLinear, 285
HistoryMalusMulQuad, 201
HistoryReductionDiv, 7654
RazoringBase, 630
RazoringQuadMul, 305
SEEPawn, 99
SEEKnight, 329
SEEBishop, 296
SEERook, 510
SEEQueen, 955
DeltaPruningPawnBonus, 140
Tempo, 2
SoftTMRatio, 27
HardTMRatio, 2
NodeTMBase, 223
NodeTMMul, 112
LMRFutilityBase, 56
LMRFutilityMultiplier, 48
CaptLMRBase, 25
CaptLMRMultiplier, 27
*/

SearchParam reverse_futility_margin("ReverseFutilityMargin", 46, 30, 100, 10);

SearchParam null_move_base("NullMoveBase", 4, 1, 6, 1);
SearchParam null_move_divisor("NullMoveDivisor", 3, 1, 8, 1);

SearchParam late_move_reduction_base("LateMoveReductionBase", 89, 30, 120, 15);
SearchParam late_move_reduction_multiplier("LateMoveReductionMultiplier", 48, 10, 70, 7);
SearchParam late_move_reduction_corrplexity("LateMoveReductionCorrplexity", 98, 50, 150, 7);

SearchParam late_move_pruning_base("LateMovePruningBase", 4, 1, 10, 1);
SearchParam late_move_pruning_quad("LateMovePruningQuad", 1, 1, 10, 1);

SearchParam futility_eval_base("FutilityEvalBase", 97, 50, 200, 15);
SearchParam futility_depth_mul("FutilityDepthMul", 90, 50, 200, 12);

SearchParam aspiration_window_delta("AspirationWindowDelta", 11, 5, 30, 2);
SearchParam aspiration_widening_factor("AspirationWideningFactor", 18, 1, 200, 10);

SearchParam see_noisy_margin("SeeNoisyMargin", -101, -120, -30, 6);
SearchParam see_quiet_margin("SeeQuietMargin", -45, -120, -30, 6);

SearchParam quiet_history_pruning_quad("QuietHistoryPruningQuad", 2028, 786, 8192, 64);

SearchParam history_bonus_base("HistoryBonusBase", -51, -384, 768, 64);
SearchParam history_bonus_mul_linear("HistoryBonusMulLinear", 205, 64, 384, 32);
SearchParam history_bonus_mul_quad("HistoryBonusMulQuad", 411, 1, 1536, 64);

SearchParam history_malus_base("HistoryMalusBase", 56, -384, 768, 64);
SearchParam history_malus_mul_linear("HistoryMalusMulLinear", 285, 64, 384, 32);
SearchParam history_malus_mul_quad("HistoryMalusMulQuad", 201, 1, 1536, 64);

SearchParam history_reduction_div("HistoryReductionDiv", 7654, 1024, 16384, 512);

SearchParam razoring_base("RazoringBase", 630, -384, 768, 15);
SearchParam razoring_quad_mul("RazoringQuadMul", 305, 1, 1536, 15);

SearchParam see_pawn("SEEPawn", 99, 70, 130, 15);
SearchParam see_knight("SEEKnight", 329, 250, 450, 15);
SearchParam see_bishop("SEEBishop", 296, 250, 450, 15);
SearchParam see_rook("SEERook", 510, 400, 650, 15);
SearchParam see_queen("SEEQueen", 955, 800, 1200, 15);

SearchParam delta_pruning_pawn_bonus("DeltaPruningPawnBonus", 140, 80, 200, 20);

SearchParam tempo("Tempo", 2, 0, 20, 2);

SearchParam soft_tm_ratio("SoftTMRatio", 27, 5, 50, 8);
SearchParam hard_tm_ratio("HardTMRatio", 2, 1, 20, 4);
SearchParam node_tm_base("NodeTMBase", 223, 50, 300, 20);
SearchParam node_tm_mul("NodeTMMul", 112, 50, 300, 20);

SearchParam lmr_futility_base("LMRFutilityBase", 56, 20, 80, 15);
SearchParam lmr_futility_multiplier("LMRFutilityMultiplier", 48, 20, 80, 7);

SearchParam capt_lmr_base("CaptLMRBase", 25, 15, 50, 15);
SearchParam capt_lmr_multiplier("CaptLMRMultiplier", 27, 10, 40, 7);