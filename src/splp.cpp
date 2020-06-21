//
// Created by Ivan Stolov on 19.06.2020.
//

#include "splp.h"

#include <utility>

unsigned int splp::get_plants_num() const {
    return plants_num;
}

unsigned int splp::get_customers_num() const {
    return customers_num;
}

float splp::get_from_transport_matrix(unsigned int i, unsigned int j) const {
    return transport_cost_matrix[i][j];
}

float splp::get_from_fixed_cost_matrix(unsigned int i) const {
    return fixed_cost_matrix[i];
}

std::vector<std::vector<float>> splp::get_transport_matrix() {
    return transport_cost_matrix;
}

std::vector<float> splp::get_fixed_cost_matrix() {
    return fixed_cost_matrix;
}

void splp::set_minimum_cost(double cost) {
    minimum_cost = cost;
}

double splp::get_minimum_cost() const {
    return minimum_cost;
}

void splp::set_possible_plants_to_open(std::vector<unsigned int> answer) {
    possible_plants_to_open = std::move(answer);
}

std::vector<unsigned int> splp::get_possible_plants_to_open() {
    return possible_plants_to_open;
}

std::istream &operator>>(std::istream &is, splp &problem_sample) {
    unsigned int m, n;
    is >> m;
    is >> n;

    problem_sample.plants_num = m;
    problem_sample.customers_num = n;

    problem_sample.fixed_cost_matrix.reserve(m);
    for (int i = 0; i < m; ++i) {
        // Capacity of warehouse. Not used in uncapacitated problem
        float not_used;
        is >> not_used;

        float current_cost;
        is >> current_cost;
        problem_sample.fixed_cost_matrix.push_back(current_cost);
    }

    problem_sample.transport_cost_matrix.resize(n);
    for (int i = 0; i < n; ++i) {
        // Customer's demand. Not used in uncapacitated problem
        float not_used;
        is >> not_used;

        problem_sample.transport_cost_matrix[i].reserve(m);
        for (int j = 0; j < m; ++j) {
            float current_val;
            is >> current_val;
            problem_sample.transport_cost_matrix[i].push_back(current_val);
        }
    }
    return is;
}

std::ostream &operator<<(std::ostream &os, splp &problem_sample) {
    //TODO
    return os;
}
