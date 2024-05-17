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

std::pair<int, int> FindMin(const std::vector<std::vector<double>>& array) {
    int best_i = 0, best_j = 0;
    for (int i = 0; i < array.size(); ++i) {
        for (int j = 0; j < array[0].size(); ++j) {
            if (array[i][j] < array[best_i][best_j]) {
                best_i = i;
                best_j = j;
            }
        }
    }
    return {best_i, best_j};
}

int main() {
    const std::string kFunctionType = "exp";
    const int kAllowNoAircrafts = 1;
    const int kExperimentsNumber = 15;
    std::vector<int> init_temperatures = {10, 100, 500, 1000, 1500, 2000};
    std::vector<double> alphas = {0.2, 0.4, 0.6, 0.8, 0.9, 0.95, 1, 2, 5};
    const int kMaxTestNumber = 5;

    std::mt19937 generator(56907);
    std::uniform_int_distribution<int> seed_dist(1, 1'000'000);

    for (int test_size = 0; test_size <= kMaxTestNumber; ++test_size) {
        std::cout << "test size " << test_size << '\n';
        std::vector<std::vector<double>> total_fines(
            init_temperatures.size(), std::vector<double>(alphas.size(), 0)
        );
        std::vector<std::vector<int>> total_successes(
            init_temperatures.size(), std::vector<int>(alphas.size(), 0)
        );

        double total_time = 0;
        for (int iteration = 0; iteration < kExperimentsNumber; ++iteration) {
            std::string input_filename = "../input_data/input" + std::to_string(test_size) + ".json";
            std::string data_filename = "../data/test" + std::to_string(test_size)
                + "_" + std::to_string(iteration) + ".json";
            for (int init_temp = 0; init_temp < init_temperatures.size(); ++init_temp) {
                for (int alpha = 0; alpha < alphas.size(); ++alpha) {
                    // std::cout << init_temperatures[init_temp] << ' ' << alphas[alpha] << std::endl;
                    boost::process::ipstream is1; //reading pipe-stream
                    boost::process::ipstream error; //reading pipe-stream

                    auto args = std::to_string(seed_dist(generator)) + " "
                        + std::to_string(kAllowNoAircrafts) + " " + kFunctionType + " "
                        + std::to_string(init_temperatures[init_temp]) + " "
                        + std::to_string(alphas[alpha]) + " "
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
                    total_fines[init_temp][alpha] += result;
                    if (result < constants::SEATS_FINE) {
                        total_successes[init_temp][alpha] += 1;
                    }
                    total_time += std::chrono::duration<double>(finish - start).count();
                }
            }
        }
        double avg_time = total_time / (kExperimentsNumber * init_temperatures.size() * alphas.size());
        std::cout << "average time = " << avg_time << std::endl;

        auto best_indices = FindMin(total_fines);
        std::cout << "initial temperature: " << init_temperatures[best_indices.first] << '\n';
        std::cout << "alpha: " << alphas[best_indices.second] << '\n';
        std::cout << "fine: " << total_fines[best_indices.first][best_indices.second] << "\n\n";
        for (auto& array : total_successes) {
            for (auto item : array) {
                std::cout << item << ' ';
            }
            std::cout << std::endl;
        }
    }
    return 0;
}
