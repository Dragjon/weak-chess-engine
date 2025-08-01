#pragma once
#include <cstdint>
#include <string>
#include <algorithm>
#include <iostream>
#include <vector>

// Global container for all parameters
inline std::vector<struct SearchParam*> all_params;

struct SearchParam {
    std::string name;
    int32_t current;
    int32_t min;
    int32_t max;
    int32_t step;

    // Constructor
    SearchParam(const std::string& name, int32_t current, int32_t min, int32_t max, int32_t step)
        : name(name), current(current), min(min), max(max), step(step) {
        all_params.push_back(this); // Register this param
    }

    void set(int32_t value) {
        current = std::clamp(value, min, max);
    }

    std::string to_string() const {
        return name + ": current=" + std::to_string(current) +
               ", min=" + std::to_string(min) +
               ", max=" + std::to_string(max) +
               ", step=" + std::to_string(step);
    }

    void print_uci_option() const {
        std::cout << "option name " << name
                  << " type spin"
                  << " default " << current
                  << " min " << min
                  << " max " << max
                  << std::endl;
    }
};

// Print all options in UCI format
inline void print_all_uci_options() {
    for (const auto* param : all_params)
        param->print_uci_option();
}

// Prints the OpenBench SPSA config
inline void print_open_bench_config()
{
    for (const auto& param : all_params)
    {
        if (param->name == "Threads" || param->name == "Hash")
            continue;

        std::cout << param->name << ", int, "
                  << param->current << ", "
                  << param->min << ", "
                  << param->max << ", "
                  << param->step << ", "
                  << "0.002" << std::endl;
    }
}


extern SearchParam tt_size;
extern SearchParam threads;
extern SearchParam move_overhead;
extern SearchParam reverse_futility_margin;
extern SearchParam null_move_depth;
extern SearchParam null_move_base;
extern SearchParam null_move_divisor;
extern SearchParam late_move_reduction_base;
extern SearchParam late_move_reduction_multiplier;
extern SearchParam late_move_reduction_corrplexity;
extern SearchParam late_move_pruning_base;
extern SearchParam late_move_pruning_quad;
extern SearchParam futility_eval_base;
extern SearchParam futility_depth_mul;
extern SearchParam aspiration_window_delta;
extern SearchParam aspiration_widening_factor;
extern SearchParam see_noisy_margin;
extern SearchParam see_quiet_margin;
extern SearchParam quiet_history_pruning_quad;
extern SearchParam history_bonus_base;
extern SearchParam history_bonus_mul_linear;
extern SearchParam history_bonus_mul_quad;
extern SearchParam history_malus_base;
extern SearchParam history_malus_mul_linear;
extern SearchParam history_malus_mul_quad;
extern SearchParam history_reduction_div;
extern SearchParam razoring_base;
extern SearchParam razoring_quad_mul;
extern SearchParam see_pawn;
extern SearchParam see_knight;
extern SearchParam see_bishop;
extern SearchParam see_rook;
extern SearchParam see_queen;
extern SearchParam delta_pruning_pawn_bonus;
extern SearchParam tempo;
extern SearchParam soft_tm_ratio;
extern SearchParam hard_tm_ratio;
extern SearchParam node_tm_base;
extern SearchParam node_tm_mul;