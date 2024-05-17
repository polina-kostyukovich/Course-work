#include <fstream>
#include <iostream>
#include <random>

#include <boost/process.hpp>

#include "../constants/heuristic_constants.h"

double GetResult(boost::process::child& solution,
                 boost::process::ipstream& error) {
    std::string line;
    double result = 0;
    while (solution.running() && std::getline(error, line)) {
        if (!line.empty()) {
            // std::cout << line << std::endl;
            result = std::stod(line);
        }
    }
    solution.wait();
    // if (result < constants::SEATS_FINE) {
    //     std::cout << "FEASIBLE" << std::endl;
    // }
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

struct SimulatedAnnealingArgs {
    std::string name;
    int allow_no_aircraft;
    double init_temp;
    double alpha;
};

int main() {
    const int kExperimentsNumber = 15;
    const int kTestNumber = 5;
    std::mt19937 generator(42);
    std::uniform_int_distribution<int> seed_dist(1, 1'000'000);

    std::vector<SimulatedAnnealingArgs> sim_ann_args = {
        {"log", 0, 1500, 0.9},
        {"log", 1, 2000, 0.95},
        {"log", 1, 100, 0.6},
        {"log", 0, 2000, 0.8},
        {"log", 0, 10, 0.6},
        {"log", 0, 2000, 0.6},
    };
    std::vector<int> local_search_allows = {1, 0, 1, 0, 0, 0};

    for (int test_size = kTestNumber; test_size <= kTestNumber; ++test_size) {
        std::cout << "test size = " << test_size << '\n';
        std::vector<double> total_fines(3, 0);
        std::vector<int> successes_number(3, 0);

        double total_time_ilp = 0;
        double total_time_local = 0;
        double total_time_sim_ann = 0;
        for (int iteration = 0; iteration < kExperimentsNumber; ++iteration) {
            boost::process::ipstream is1; //reading pipe-stream
            boost::process::ipstream is2; //reading pipe-stream
            boost::process::ipstream is3; //reading pipe-stream

            std::string input_filename = "../input_data/input" + std::to_string(test_size) + ".json";
            std::string data_filename = "../data/test" + std::to_string(test_size)
                + "_" + std::to_string(iteration) + ".json";

            // ilp
            boost::process::ipstream error1; //reading pipe-stream
            std::cout << "ilp solution" << std::endl;
            auto args = input_filename + " " + data_filename;
            auto start = std::chrono::high_resolution_clock::now();
            auto ilp_solution = boost::process::child("ilp_solution",
                                                      args,
                                                      boost::process::std_out > is1,
                                                      boost::process::std_err > error1);
            auto finish = std::chrono::high_resolution_clock::now();
            double result = GetResult(ilp_solution, error1);
            total_fines[0] += result * constants::FLIGHTS_COST;
            if (result * constants::FLIGHTS_COST < constants::SEATS_FINE) {
                std::cout << "FEASIBLE" << std::endl;
                successes_number[0] += 1;
            }
            total_time_ilp += std::chrono::duration<double>(finish - start).count();

            // local search
            boost::process::ipstream error2; //reading pipe-stream
            std::cout << "local search solution" << std::endl;
            args = std::to_string(seed_dist(generator)) + " "
                + std::to_string(local_search_allows[test_size]) + " "
                + input_filename + " "
                + data_filename;
            start = std::chrono::high_resolution_clock::now();
            auto local_search_solution =
                boost::process::child("local_search_solution",
                                      args,
                                      boost::process::std_out > is2,
                                      boost::process::std_err > error2);
            finish = std::chrono::high_resolution_clock::now();
            result = GetResult(local_search_solution, error2);
            total_fines[1] += result;
            if (result < constants::SEATS_FINE) {
                std::cout << "FEASIBLE" << std::endl;
                successes_number[1] += 1;
            }
            total_time_local += std::chrono::duration<double>(finish - start).count();

            // simulated annealing
            boost::process::ipstream error3; //reading pipe-stream
            std::cout << "simulated annealing solution" << std::endl;
            args = std::to_string(seed_dist(generator)) + " "
                + std::to_string(sim_ann_args[test_size].allow_no_aircraft) + " "
                + sim_ann_args[test_size].name + " "
                + std::to_string(sim_ann_args[test_size].init_temp) + " "
                + std::to_string(sim_ann_args[test_size].alpha) + " "
                + input_filename + " "
                + data_filename;
            start = std::chrono::high_resolution_clock::now();
            auto simulated_annealing_solution =
                boost::process::child("simulated_annealing_solution",
                                      args,
                                      boost::process::std_out > is3,
                                      boost::process::std_err > error3);
            finish = std::chrono::high_resolution_clock::now();
            result = GetResult(simulated_annealing_solution, error3);
            total_fines[2] += result;
            if (result < constants::SEATS_FINE) {
                std::cout << "FEASIBLE" << std::endl;
                successes_number[2] += 1;
            }
            total_time_sim_ann += std::chrono::duration<double>(finish - start).count();
        }
        double avg_time_ilp = total_time_ilp / kExperimentsNumber;
        double avg_time_local = total_time_local / kExperimentsNumber;
        double avg_time_sim_ann = total_time_sim_ann / kExperimentsNumber;
        std::cout << "average time ilp: " << avg_time_ilp << '\n';
        std::cout << "average time local search: " << avg_time_local << '\n';
        std::cout << "average time simulated annealing: " << avg_time_sim_ann << '\n';

        std::cout << "fines: ";
        for (auto fine : total_fines) {
            std::cout << fine << ' ';
        }
        std::cout << '\n';
        std::cout << "successes: ";
        for (auto success : successes_number) {
            std::cout << success << ' ';
        }
        std::cout << '\n';

        auto best_index = FindMin(total_fines);
        std::cout << "best solution: " << best_index << '\n';
        std::cout << "fine: " << total_fines[best_index] << "\n\n";
    }
    return 0;
}
