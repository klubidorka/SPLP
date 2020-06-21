//
// Created by Ivan Stolov on 19.06.2020.
//

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <functional>


#include "splp.h"

#ifndef SPLP_SPLP_SOLVER_H
#define SPLP_SPLP_SOLVER_H

struct hash_vector {
    size_t operator()(const std::vector<float>& p) const
    {
        auto hash = std::hash<float>{}(p[0]);
        for (auto it : p) {
            hash ^= std::hash<float>{}(it);
        }
        return hash;
    }
};

class splp_solver {
private:
    splp &problem;
    std::unordered_map<std::vector<float>, float, hash_vector> hammer_function;
    float hammer_function_constant = 0;
    std::vector<std::vector<unsigned int>> permutations;

    std::vector<bool> is_plant_to_open;
    std::vector<bool> is_plant_finalized;

    std::vector<unsigned int> phase_time_spent_millis;

    const float HEURISTIC_1_ALPHA = 0.065;
    const float SOLVER_BETA = 1;

    double lower_bound;
    double answer;
    bool is_solved;

    int khumawala_opened_plants = 0;
    int khumawala_closed_plants = 0;
    int heuristic_1_opened_plants = 0;
    int heuristic_2_opened_plants = 0;

    void prepare_permutation_matrix(const std::vector<std::vector<float>> &transportation_cost);

    void calculate_lower_bound();

    void update_hammer_function();

    void calculate_hammer_function(const std::vector<std::vector<float>> &transportation_cost);

    void apply_khumawala_rules();

    void apply_heuristic1();

    void apply_heuristic2();

    void next_permutation(std::vector<bool> &permutation);

    float calculate_hammer_function_value();

    void blind_search();

public:
    void solve();

    double get_min_hammer_function() const;

    unsigned int get_execution_time() const;

    std::vector<unsigned int> get_plants_to_open();

    double get_max_eps_error() const;

    explicit splp_solver(splp &problem);
};

#endif //SPLP_SPLP_SOLVER_H
