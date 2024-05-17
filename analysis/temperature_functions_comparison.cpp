#include <fstream>
#include <iostream>
#include <random>

#include <boost/process.hpp>

#include "../constants/heuristic_constants.h"

double GetResult(boost::process::child& solution,
                 boost::process::ipstream& is,
                 boost::process::ipstream& error) {
    std::string line;
    double result = 0;
    while (solution.running() && std::getline(error, line)) {
        if (!line.empty()) {
            result = std::stod(line);
        }
    }
    solution.wait();
    if (result < constants::SEATS_FINE) {
        std::cout << "FEASIBLE" << std::endl;
    }
    return result;
}

int FindMin(const std::vector<double>& array) {
    int best_i = 0;
    for (int i = 0; i < array.size(); ++i) {
        if (array[i] < array[best_i]) {
            best_i = i;
        }
    }
    return best_i;
}

struct Params {
    double init_temp;
    double alpha;
};

struct TempFuncInfo {
    std::string name;
    std::vector<Params> params;
};

int main() {
    const int kAllowNoAircrafts = 0;
    const int kExperimentsNumber = 15;
    const int kMaxTestNumber = 5;
    // std::vector<TempFuncInfo> temp_funcs = {  // allow == false
    //     {"log", {{1500, 0.9}, {100, 0.95}, {100, 0.95}, {2000, 0.8}, {10, 0.6}, {2000, 0.6}}},
    //     {"exp", {{2000, 0.2}, {1500, 0.95}, {100, 0.4}, {500, 0.2}, {2000, 0.8}, {500, 0.2}}},
    //     {"geom", {{1500, 0.9}, {1500, 0.95}, {1500, 0.95}, {100, 0.8}, {2000, 0.8}, {100, 0.6}}},
    // };
    // new
    std::vector<TempFuncInfo> temp_funcs = {  // allow == false
        {"log", {{1500, 0.9}, {100, 0.95}, {100, 0.95}, {2000, 0.8}, {10, 0.6}, {2000, 0.6}}},
        {"exp", {{10, 2}, {500, 2}, {100, 1}, {500, 2}, {2000, 0.8}, {1000, 1}}},
        {"geom", {{1500, 0.9}, {1500, 0.95}, {1500, 0.95}, {100, 0.8}, {2000, 0.8}, {100, 0.6}}},
    };

    // std::vector<TempFuncInfo> temp_funcs = {  // allow == true
    //     {"log", {{100, 0.9}, {2000, 0.95}, {100, 0.6}, {100, 0.6}, {10, 0.6}, {1000, 0.6}}},
    //     {"exp", {{100, 0.9}, {2000, 0.9}, {500, 0.95}, {1000, 0.2}, {100, 0.8}, {1000, 0.8}}},
    //     {"geom", {{500, 0.95}, {100, 0.95}, {2000, 0.95}, {1000, 0.2}, {10, 0.95}, {1000, 0.8}}}
    // };
    // new
    // std::vector<TempFuncInfo> temp_funcs = {  // allow == true
    //     {"log", {{100, 0.9}, {2000, 0.95}, {100, 0.6}, {100, 0.6}, {10, 0.6}, {1000, 0.6}}},
    //     {"exp", {{100, 0.9}, {2000, 0.9}, {500, 0.95}, {1000, 0.2}, {100, 0.8}, {1000, 0.8}}},
    //     {"geom", {{500, 0.95}, {100, 0.95}, {2000, 0.95}, {1000, 5}, {10, 0.95}, {1000, 0.8}}}
    // };

    std::mt19937 generator(5690987);
    std::uniform_int_distribution<int> seed_dist(1, 1'000'000);

    for (int test_size = 0; test_size <= kMaxTestNumber; ++test_size) {
        std::cout << "test size = " << test_size << '\n';
        std::vector<double> total_fines(3, 0);

        for (int i = 0; i < temp_funcs.size(); ++i) {
            double total_time = 0;
            auto& temp_func = temp_funcs[i];
            for (int iteration = 0; iteration < kExperimentsNumber; ++iteration) {
                std::string input_filename = "../input_data/input" + std::to_string(test_size) + ".json";
                std::string data_filename = "../data/test" + std::to_string(test_size)
                    + "_" + std::to_string(iteration) + ".json";

                boost::process::ipstream is1; //reading pipe-stream
                boost::process::ipstream error; //reading pipe-stream

                auto args = std::to_string(seed_dist(generator)) + " "
                    + std::to_string(kAllowNoAircrafts) + " " + temp_func.name + " "
                    + std::to_string(temp_func.params[test_size].init_temp) + " "
                    + std::to_string(temp_func.params[test_size].alpha) + " "
                    + input_filename + " "
                    + data_filename;
                auto start = std::chrono::high_resolution_clock::now();
                auto solution =
                    boost::process::child("simulated_annealing_solution",
                                          args,
                                          boost::process::std_out > is1,
                                          boost::process::std_err > error);
                auto finish = std::chrono::high_resolution_clock::now();
                auto result = GetResult(solution, is1, error);
                total_fines[i] += result;
                total_time += std::chrono::duration<double>(finish - start).count();
            }
            double avg_time = total_time / kExperimentsNumber;
            std::cout << "function " << temp_func.name << ", average time = " << avg_time << '\n';
        }

        std::cout << "fines: ";
        for (auto fine : total_fines) {
            std::cout << fine << ' ';
        }
        std::cout << '\n';

        auto best_index = FindMin(total_fines);
        std::cout << "best function: " << temp_funcs[best_index].name << '\n';
        std::cout << "fine: " << total_fines[best_index] << "\n\n";
    }
    return 0;
}
