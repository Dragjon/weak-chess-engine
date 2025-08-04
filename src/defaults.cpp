#include "defaults.hpp"

// Basics
SearchParam tt_size("Hash", 64, 1, 16384, 1);
SearchParam threads("Threads", 1, 1, 1, 1);
SearchParam move_overhead("Move Overhead", 0, 0, 10000, 1);

// SPSA (https://chess.n9x.co/tune/3904/)
/*
ReverseFutilityMargin, 52
NullMoveBase, 4
NullMoveDivisor, 4
LateMoveReductionBase, 73
LateMoveReductionMultiplier, 56
LateMoveReductionCorrplexity, 91
LateMovePruningBase, 3
LateMovePruningQuad, 3
FutilityEvalBase, 101
FutilityDepthMul, 102
AspirationWindowDelta, 10
AspirationWideningFactor, 33
SeeNoisyMargin, -100
SeeQuietMargin, -58
QuietHistoryPruningQuad, 2035
HistoryBonusBase, 41
HistoryBonusMulLinear, 228
HistoryBonusMulQuad, 389
HistoryMalusBase, -35
HistoryMalusMulLinear, 262
HistoryMalusMulQuad, 138
HistoryReductionDiv, 8178
RazoringBase, 604
RazoringQuadMul, 347
SEEPawn, 102
SEEKnight, 334
SEEBishop, 308
SEERook, 509
SEEQueen, 924
DeltaPruningPawnBonus, 132
Tempo, 6
SoftTMRatio, 23
HardTMRatio, 2
NodeTMBase, 217
NodeTMMul, 103
SEDoubleExtMargin, 16
CorrectionDivisor, 2027
BMScale0, 237
BMScale1, 132
BMScale2, 118
BMScale3, 83
BMScale4, 53
ScoreScale0, 121
ScoreScale1, 109
ScoreScale2, 100
ScoreScale3, 87
ScoreScale4, 99
PawnCorrhistWeight, 206
NonPawnCorrhistWeight, 168
MinorCorrhistWeight, 157
MajorCorrhistWeight, 129
*/

SearchParam reverse_futility_margin("ReverseFutilityMargin", 52, 30, 100, 10);

SearchParam null_move_base("NullMoveBase", 4, 1, 6, 1);
SearchParam null_move_divisor("NullMoveDivisor", 4, 1, 8, 1);

SearchParam late_move_reduction_base("LateMoveReductionBase", 73, 30, 120, 15);
SearchParam late_move_reduction_multiplier("LateMoveReductionMultiplier", 56, 10, 70, 7);
SearchParam late_move_reduction_corrplexity("LateMoveReductionCorrplexity", 91, 50, 150, 7);

SearchParam late_move_pruning_base("LateMovePruningBase", 3, 1, 10, 1);
SearchParam late_move_pruning_quad("LateMovePruningQuad", 3, 1, 10, 1);

SearchParam futility_eval_base("FutilityEvalBase", 101, 50, 200, 15);
SearchParam futility_depth_mul("FutilityDepthMul", 102, 50, 200, 6);

SearchParam aspiration_window_delta("AspirationWindowDelta", 10, 5, 30, 2);
SearchParam aspiration_widening_factor("AspirationWideningFactor", 33, 1, 200, 20);

SearchParam see_noisy_margin("SeeNoisyMargin", -100, -120, -30, 6);
SearchParam see_quiet_margin("SeeQuietMargin", -58, -120, -30, 6);

SearchParam quiet_history_pruning_quad("QuietHistoryPruningQuad", 2035, 786, 8192, 64);

SearchParam history_bonus_base("HistoryBonusBase", 41, -384, 768, 64);
SearchParam history_bonus_mul_linear("HistoryBonusMulLinear", 228, 64, 384, 32);
SearchParam history_bonus_mul_quad("HistoryBonusMulQuad", 389, 1, 1536, 64);

SearchParam history_malus_base("HistoryMalusBase", -35, -384, 768, 64);
SearchParam history_malus_mul_linear("HistoryMalusMulLinear", 262, 64, 384, 32);
SearchParam history_malus_mul_quad("HistoryMalusMulQuad", 138, 1, 1536, 64);

SearchParam history_reduction_div("HistoryReductionDiv", 8178, 1024, 16384, 128);

SearchParam razoring_base("RazoringBase", 604, -384, 768, 64);
SearchParam razoring_quad_mul("RazoringQuadMul", 347, 1, 1536, 64);

SearchParam see_pawn("SEEPawn", 102, 70, 130, 10);
SearchParam see_knight("SEEKnight", 334, 250, 450, 15);
SearchParam see_bishop("SEEBishop", 308, 250, 450, 15);
SearchParam see_rook("SEERook", 509, 400, 650, 15);
SearchParam see_queen("SEEQueen", 924, 800, 1200, 15);

SearchParam delta_pruning_pawn_bonus("DeltaPruningPawnBonus", 132, 80, 200, 20);

SearchParam tempo("Tempo", 6, 0, 20, 3);

SearchParam soft_tm_ratio("SoftTMRatio", 23, 5, 50, 8);
SearchParam hard_tm_ratio("HardTMRatio", 2, 1, 20, 4);
SearchParam node_tm_base("NodeTMBase", 217, 50, 300, 20);
SearchParam node_tm_mul("NodeTMMul", 103, 50, 300, 20);

SearchParam se_double_ext_margin("SEDoubleExtMargin", 16, 0, 60, 5);

SearchParam correction_divisor("CorrectionDivisor", 2027, 1024, 4096, 64);

SearchParam bm_scale0("BMScale0", 237, 100, 500, 20);
SearchParam bm_scale1("BMScale1", 132, 50, 300, 20);
SearchParam bm_scale2("BMScale2", 118, 25, 250, 20);
SearchParam bm_scale3("BMScale3", 83, 10, 200, 20);
SearchParam bm_scale4("BMScale4", 53, 0, 150, 20);

SearchParam score_scale0("ScoreScale0", 121, 50, 300, 20);
SearchParam score_scale1("ScoreScale1", 109, 50, 250, 20);
SearchParam score_scale2("ScoreScale2", 100, 25, 200, 20);
SearchParam score_scale3("ScoreScale3", 87, 20, 200, 20);
SearchParam score_scale4("ScoreScale4", 99, 10, 200, 20);

SearchParam pawn_corrhist_weight("PawnCorrhistWeight", 206, 80, 350, 20);
SearchParam nonpawn_corrhist_weight("NonPawnCorrhistWeight", 168, 80, 350, 20);
SearchParam minor_corrhist_weight("MinorCorrhistWeight", 157, 80, 350, 20);
SearchParam major_corrhist_weight("MajorCorrhistWeight", 129, 70, 350, 20);
