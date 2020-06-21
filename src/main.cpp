#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <unordered_map>

#include "splp.h"
#include "splp_solver.h"


const std::string SAMPLES_PATH = "../samples";
const std::string REPORT_PATH = "../reports/results.txt";
const std::string OPTIMAL_PATH = "../samples/uncapopt.txt";

std::string parse_name(const std::string &path) {
    std::string result;
    for (auto ch : path) {
        if (std::isdigit(ch)) {
            result += ch;
        }
    }
    return result;
}

void split(const std::string &str, std::unordered_map<std::string, double> &result) {
    std::string name;
    std::string value;
    int i = 3;
    while(str[i] != ' '){
        name += str[i++];
    }
    while(str[i] == ' '){
        i++;
    }
    while(i < str.size()){
        value += str[i++];
    }
    result.insert(std::make_pair(name, atof(value.c_str())));
}


void start_benchmark_with_report() {
    using std::cout;
    using std::cerr;
    using std::endl;
    using std::setw;
    using std::left;
    using std::__fs::filesystem::directory_iterator;

    std::unordered_map<std::string, double> optimal_values;
    std::ifstream optimal;
    optimal.open(OPTIMAL_PATH);

    std::string line;
    getline(optimal, line);
    while(getline(optimal, line)){
        split(line, optimal_values);
    }
    optimal.close();

    std::string path = SAMPLES_PATH;

    std::ofstream results;
    results.open(REPORT_PATH);
    results << left << setw(12) << "Data file"
            << setw(18) << "Our solution"
            << setw(22) << "Optimal solution"
            << setw(15) << "Error, %"
            << setw(15) << "Solve time, ms" << "\n\n";

    for (const auto &entry : directory_iterator(path)) {
        splp sample;
        std::string name = parse_name(entry.path());
        if (!name.empty()) {
            std::ifstream in(entry.path());
            if (in.is_open()) {
                in >> sample;
                splp_solver solver(sample);
                solver.solve();
                cout << "Solved cap" + name + "\n";

                std::string opt = "N / A";
                std::string diff = "N / A";
                if (optimal_values.find(name) != optimal_values.end()){
                    double optimal_value = optimal_values[name];

                    std::ostringstream streamObj;
                    streamObj << std::setprecision(0) << std::fixed;
                    streamObj << optimal_value;

                    opt = streamObj.str();
                    double difference =  (solver.get_min_hammer_function() - optimal_value) / optimal_value * 100;

                    std::ostringstream streamObj2;
                    streamObj2 << std::setprecision(0) << std::fixed;
                    streamObj2 << difference;

                    diff = streamObj2.str();
                }

                results << left << setw(12) << "cap" + name
                        << setw(18) << solver.get_min_hammer_function()
                        << setw(22) << opt
                        << setw(15) << diff
                        << setw(15) << solver.get_execution_time() << "\n";
            } else {
                cerr << "Cannot open file with sample" + name + "\n";
            }
        }
    }
    results.close();
}

int main() {
    start_benchmark_with_report();
}