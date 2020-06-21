//
// Created by Ivan Stolov on 19.06.2020.
//

#include <vector>
#include <iostream>

#ifndef SPLP_SPLP_H
#define SPLP_SPLP_H


class splp {
private:
    unsigned int plants_num;
    unsigned int customers_num;
    std::vector<std::vector<float>> transport_cost_matrix;
    std::vector<float> fixed_cost_matrix;

    double minimum_cost;
    std::vector<unsigned int> possible_plants_to_open;
public:
    unsigned int get_plants_num() const;

    unsigned int get_customers_num() const;

    float get_from_transport_matrix(unsigned int i, unsigned int j) const;

    float get_from_fixed_cost_matrix(unsigned int i) const;

    std::vector<std::vector<float>> get_transport_matrix();

    std::vector<float> get_fixed_cost_matrix();

    void set_minimum_cost(double cost);

    double get_minimum_cost() const;

    void set_possible_plants_to_open(std::vector<unsigned int> answer);

    std::vector<unsigned int> get_possible_plants_to_open();

    friend std::istream &operator>>(std::istream &is, splp &problem_sample);

    friend std::ostream &operator<<(std::ostream &os, splp &problem_sample);
};


#endif //SPLP_SPLP_H
