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
    SearchParam(const std::string& param_name, int32_t input_current, int32_t input_min, int32_t input_max, int32_t input_step)
        : name(param_name), current(input_current), min(input_min), max(input_max), step(input_step) {
        all_params.push_back(this);
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
inline void printOpenBenchConfig()
{
    for (const auto& param : all_params)
    {
        if (param->name == "Threads" || param->name == "Hash" || param->name == "NullMoveDepth" || param->name == "LateMoveReductionDepth" || param->name == "AspirationWindowDepth") // Skip tt_size and threads and others
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
extern SearchParam reverse_futility_margin;
extern SearchParam reverse_futility_depth;
extern SearchParam null_move_depth;
extern SearchParam null_move_reduction;
extern SearchParam late_move_reduction_depth;
extern SearchParam late_move_reduction_base;
extern SearchParam late_move_reduction_multiplier;
extern SearchParam aspiration_window_depth;
extern SearchParam aspiration_window_delta;
extern SearchParam aspiration_widening_factor;
extern SearchParam internal_iterative_reduction_depth;
extern SearchParam see_noisy_margin;
extern SearchParam see_quiet_margin;
extern SearchParam history_bonus_base;
extern SearchParam history_bonus_mul_linear;
extern SearchParam history_bonus_mul_quad;
extern SearchParam history_malus_base;
extern SearchParam history_malus_mul_linear;
extern SearchParam history_malus_mul_quad;
extern SearchParam razoring_max_depth;
extern SearchParam razoring_base;
extern SearchParam razoring_linear_mul;
extern SearchParam razoring_quad_mul;
extern SearchParam tempo;
extern SearchParam soft_tm_ratio;
extern SearchParam hard_tm_ratio;
extern SearchParam node_tm_base;
extern SearchParam node_tm_mul;
extern SearchParam see_pawn;
extern SearchParam see_knight;
extern SearchParam see_bishop;
extern SearchParam see_rook;
extern SearchParam see_queen;