//
// Created by Ivan Stolov on 19.06.2020.
//

#include "splp_solver.h"

void splp_solver::prepare_permutation_matrix(const std::vector<std::vector<float>> &transportation_cost) {
    unsigned int n = problem.get_customers_num();
    unsigned int m = problem.get_plants_num();

    permutations.resize(n);
    for (int i = 0; i < n; ++i) {
        permutations[i].reserve(m);
        for (int j = 0; j < m; ++j) {
            permutations[i].push_back(j);
        }
        const std::vector<float> &customer = transportation_cost[i];
        std::sort(permutations[i].begin(),
                  permutations[i].end(),
                  [customer](unsigned int &a, unsigned int &b) { return customer[a] <= customer[b]; });
    }
}

//TODO: improve lower bound
void splp_solver::calculate_lower_bound() {
    float current_lower_bound = 0;

    // add minimum transportation cost of every customer, since every customer should be satisfied
    for (int i = 0; i < problem.get_customers_num(); ++i) {
        current_lower_bound += problem.get_from_transport_matrix(i, permutations[i][0]);
    }

    // add minimum fixed cost, since we need to open at least one plant
    float min_fixed_cost = problem.get_from_fixed_cost_matrix(0);
    for (int j = 1; j < problem.get_plants_num(); ++j) {
        float candidate = problem.get_from_fixed_cost_matrix(j);
        if (min_fixed_cost > candidate) {
            min_fixed_cost = candidate;
        }
    }
    current_lower_bound += min_fixed_cost;

    lower_bound = current_lower_bound;
}

void splp_solver::update_hammer_function() {
    std::vector<std::pair<std::vector<float>, float>> elements_to_insert;
    auto it = hammer_function.begin();
    // erase elements that need to be deleted or modified
    while (it != hammer_function.end()) {
        std::vector<float> updated_term;
        bool delete_term = false;
        bool update_term = false;

        for (auto num : it->first) {
            if (is_plant_finalized[num]) {
                if (is_plant_to_open[num]) {
                    delete_term = true;
                    break;
                } else {
                    update_term = true;
                }
            } else {
                updated_term.push_back(num);
            }
        }
        if (delete_term || update_term) {
            if (!delete_term) {
                elements_to_insert.emplace_back(std::make_pair(updated_term, it->second));
            }
            it = hammer_function.erase(it);
        } else {
            it++;
        }
    }
    // insert modified elements
    for (const auto &elem : elements_to_insert) {
        if (elem.first.empty()) {
            hammer_function_constant += elem.second;
        } else {
            auto equal = hammer_function.find(elem.first);
            if (equal != hammer_function.end()) {
                equal->second += elem.second;
            } else {
                hammer_function.insert(elem);
            }
        }
    }
}


void splp_solver::calculate_hammer_function(const std::vector<std::vector<float>> &transportation_cost) {
    // add setting up cost
    for (int j = 0; j < problem.get_plants_num(); ++j) {
        float current_fixed_cost = problem.get_from_fixed_cost_matrix(j);
        hammer_function_constant += current_fixed_cost;
        std::vector<float> current_plant_num;
        current_plant_num.push_back(j);
        hammer_function.insert(std::make_pair<>(current_plant_num, current_fixed_cost * -1));
    }

    // add transportation cost
    for (int i = 0; i < problem.get_customers_num(); ++i) {
        const std::vector<float> &customer_cost = transportation_cost[i];
        const std::vector<unsigned int> &customer_permutation = permutations[i];

        hammer_function_constant += customer_cost[customer_permutation[0]];
        std::vector<float> current_plants_num;
        for (int j = 1; j < problem.get_plants_num(); ++j) {
            float current_delta = customer_cost[customer_permutation[j]] - customer_cost[customer_permutation[j - 1]];
            current_plants_num.push_back(customer_permutation[j - 1]);
            std::sort(current_plants_num.begin(), current_plants_num.end());
            auto equal = hammer_function.find(current_plants_num);
            if (equal != hammer_function.end()) {
                equal->second += current_delta;
            } else {
                hammer_function.insert(std::make_pair<>(current_plants_num, current_delta));
            }
        }
    }
}

void splp_solver::apply_khumawala_rules() {
    unsigned int plants_num = problem.get_plants_num();

    std::vector<float> linear_terms_costs(plants_num);
    std::vector<float> all_terms_costs(plants_num);

    // calculate sums of coefficients in Hammer function
    for (const auto &it : hammer_function) {
        if (it.first.size() == 1) {
            linear_terms_costs[it.first[0]] += it.second;
        }
        for (auto elem : it.first) {
            all_terms_costs[elem] += it.second;
        }
    }

    for (int i = 0; i < plants_num; i++) {
        if (!is_plant_finalized[i]) {
            // first Khumawala rule
            if (linear_terms_costs[i] >= 0) {
                is_plant_to_open[i] = true;
                is_plant_finalized[i] = true;
                khumawala_opened_plants++;

                // second Khumawala rule
            } else if (all_terms_costs[i] < 0) {
                is_plant_to_open[i] = false;
                is_plant_finalized[i] = true;
                khumawala_closed_plants++;
            }
        }
    }

    update_hammer_function();
}

void splp_solver::apply_heuristic1() {
    std::vector<std::pair<float, int>> costs(problem.get_plants_num());
    for (int j = 0; j < problem.get_plants_num(); ++j) {
        costs[j].second = j;
    }
    for (const auto &it : hammer_function) {
        for (auto elem : it.first) {
            costs[elem].first += it.second;
        }
    }
    std::sort(costs.begin(), costs.end(),
              [](std::pair<float, int> &a, std::pair<float, int> &b) { return a.first > b.first; });

    int plants_to_open = std::floor(problem.get_plants_num() * HEURISTIC_1_ALPHA);
    for (int i = 0; i < plants_to_open; ++i) {
        if (costs[i].first > 0) {
            is_plant_to_open[costs[i].second] = true;
            is_plant_finalized[costs[i].second] = true;
            heuristic_1_opened_plants++;
        }
    }
    update_hammer_function();
}

void splp_solver::apply_heuristic2() {
    std::unordered_set<unsigned int> y_plus;
    std::unordered_set<unsigned int> y_minus;

    for (const auto &elem : hammer_function) {
        if (elem.second >= 0) {
            for (auto it : elem.first) {
                y_plus.insert(it);
            }
        } else {
            for (auto it : elem.first) {
                y_minus.insert(it);
            }
        }
    }
    for (auto elem : y_minus) {
        y_plus.erase(elem);
    }

    heuristic_2_opened_plants = y_plus.size();

    for (auto elem : y_plus) {
        is_plant_to_open[elem] = true;
        is_plant_finalized[elem] = false;
    }

    update_hammer_function();
}

void splp_solver::next_permutation(std::vector<bool> &permutation) {
    for (int i = 0; i < permutation.size(); ++i) {
        if (!permutation[i]) {
            permutation[i] = true;
            for (int j = 0; j < i; ++j) {
                permutation[j] = false;
            }
            break;
        }
    }
}

//TODO: implement it
float splp_solver::calculate_hammer_function_value() {
    return 0;
}

//TODO optimize this function and add output
void splp_solver::blind_search() {
    int plants_left = problem.get_plants_num() -
                      (heuristic_1_opened_plants + heuristic_2_opened_plants + khumawala_closed_plants +
                       khumawala_opened_plants);
    std::vector<bool> plants_left_configuration(plants_left);
    std::vector<unsigned int> plants_left_indexes;

    plants_left_indexes.reserve(plants_left);
    for (int i = 0; i < is_plant_finalized.size(); i++) {
        if (!is_plant_finalized[i]) {
            plants_left_indexes.push_back(i);
        }
    }
    float minimal_value = hammer_function_constant;
    std::vector<bool> best_configuration = plants_left_configuration;

    assert(plants_left_indexes.size() == plants_left);
    for (unsigned int j = 0; j < (2 << (plants_left - 1)); ++j) {
        next_permutation(plants_left_configuration);
        float current_value = calculate_hammer_function_value();
        if (current_value < minimal_value){
            minimal_value = current_value;
            best_configuration = plants_left_configuration;
        }
    }
}


void splp_solver::solve() {
    // phase 0 -- preparation
    std::chrono::time_point<std::chrono::system_clock> phase0 = std::chrono::system_clock::now();
    prepare_permutation_matrix(problem.get_transport_matrix());

    calculate_lower_bound();
    calculate_hammer_function(problem.get_transport_matrix());

    // phase 1 -- use Khumawala rules and heuristics
    std::chrono::time_point<std::chrono::system_clock> phase1 = std::chrono::system_clock::now();
    phase_time_spent_millis.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(phase1 - phase0).count());

    while (heuristic_1_opened_plants + heuristic_2_opened_plants + khumawala_closed_plants + khumawala_opened_plants <
           problem.get_plants_num() * SOLVER_BETA) {
        apply_khumawala_rules();
        apply_heuristic1();
        apply_heuristic2();
    }

    // phase 2 -- blind-search through all the options left
    //TODO add blind search
    std::chrono::time_point<std::chrono::system_clock> phase2 = std::chrono::system_clock::now();
    phase_time_spent_millis.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(phase2 - phase1).count());

    is_solved = true;
    answer = hammer_function_constant;
    int index = 0;
}

double splp_solver::get_min_hammer_function() const {
    if (!is_solved) {
        std::cerr << "Result queried before solving" << std::endl;
        return -1;
    }
    return answer;
}

unsigned int splp_solver::get_execution_time() const {
    unsigned int result = 0;
    for (auto it : phase_time_spent_millis){
        result += it;
    }
    return result;
}

std::vector<unsigned int> splp_solver::get_plants_to_open() {
    if (!is_solved) {
        std::cerr << "Result queried before solving" << std::endl;
    }
    std::vector<unsigned int> plants_to_open;
    unsigned int i = 1;
    for (auto it : is_plant_to_open) {
        if (it) {
            plants_to_open.push_back(i);
        }
        i++;
    }
    return plants_to_open;
}

double splp_solver::get_max_eps_error() const {
    if (!is_solved) {
        std::cerr << "Result queried before solving" << std::endl;
        return -1;
    }
    return answer / lower_bound - 1;
}

splp_solver::splp_solver(splp &problem) : problem(problem),
                                          lower_bound(0),
                                          answer(-1),
                                          is_solved(false) {
    unsigned int plant_num = problem.get_plants_num();
    is_plant_to_open.resize(plant_num);
    is_plant_finalized.resize(plant_num);
}
