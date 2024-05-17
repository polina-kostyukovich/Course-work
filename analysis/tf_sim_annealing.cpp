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

struct TempFuncInfo {
    std::string name;
    double init_temp;
    double alpha;
};

int main() {
    const int kExperimentsNumber = 15;
    const int kMaxTestNumber = 5;
    std::vector<std::vector<TempFuncInfo>> temp_funcs = {
        {  // false
            {"log", 1500, 0.9},  // best functions for each size
            {"log", 100, 0.95},
            {"log", 100, 0.95},
            {"log", 2000, 0.8},
            {"log", 10, 0.6},
            {"log", 2000, 0.6},
        },
        {  // true
            {"log", 100, 0.9},
            {"log", 2000, 0.95},
            {"log", 100, 0.6},
            {"log", 100, 0.6},
            {"log", 10, 0.6},
            {"log", 1000, 0.6},
        }
    };

    std::mt19937 generator(29);
    std::uniform_int_distribution<int> seed_dist(1, 1'000'000);

    for (int test_size = 0; test_size <= kMaxTestNumber; ++test_size) {
        std::cout << "test size = " << test_size << '\n';
        std::vector<double> total_fines(2, 0);

        for (int allow_no_aircraft = 0; allow_no_aircraft <= 1; ++allow_no_aircraft) {
            double total_time = 0;
            auto& temp_func = temp_funcs[allow_no_aircraft][test_size];
            for (int iteration = 0; iteration < kExperimentsNumber; ++iteration) {
                std::string input_filename = "../input_data/input" + std::to_string(test_size) + ".json";
                std::string data_filename = "../data/test" + std::to_string(test_size)
                    + "_" + std::to_string(iteration) + ".json";

                boost::process::ipstream is1; //reading pipe-stream
                boost::process::ipstream error; //reading pipe-stream

                auto args = std::to_string(seed_dist(generator)) + " "
                    + std::to_string(allow_no_aircraft) + " "
                    + temp_func.name + " "
                    + std::to_string(temp_func.init_temp) + " "
                    + std::to_string(temp_func.alpha) + " "
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
                total_fines[allow_no_aircraft] += result;
                total_time += std::chrono::duration<double>(finish - start).count();
            }
            double avg_time = total_time / kExperimentsNumber;
            std::cout << "allow = " << allow_no_aircraft
                << ", function = " << temp_func.name << ", average time = " << avg_time << '\n';
        }

        std::cout << "fines: ";
        for (auto fine : total_fines) {
            std::cout << fine << ' ';
        }
        std::cout << '\n';

        auto best_index = FindMin(total_fines);
        std::cout << "best allow: " << best_index << '\n';
        std::cout << "fine: " << total_fines[best_index] << "\n\n";
    }
    return 0;
}
