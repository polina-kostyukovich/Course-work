#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include "constants/heuristic_constants.h"
#include "help_structs/data_reader.h"
#include "help_structs/entities.h"
#include "help_structs/temperature_functions.h"
#include "heuristic_model/simulated_annealing_solver.h"

void PrintSolution(const std::pair<std::vector<int>, double>& solution,
                   const SolutionCorrectnessInfo& status,
                   int aircrafts_number,
                   std::ostream& out) {
    if (status.IsCorrect()) {
        out << "Feasible solution\n";
    } else {
        out << "Infeasible solution: ";
        if (status.has_no_aircraft) {
            out << "at least one aircraft is not set\n";
        } else if (status.flights_intersect) {
            out << "flights intersect\n";
        } else if (status.airports_mismatch) {
            out << "airports mismatch\n";
        } else if (status.seats_lack) {
            out << "seats lack\n";
        }
    }
    out << "Fine = " << solution.second << '\n';

    out << "Solution:\n";
    out << "Flight" << '\t' << "Aircraft" << '\n';
    for (int i = 0; i < solution.first.size(); ++i) {
        if (solution.first[i] < aircrafts_number) {
            out << i << '\t' << solution.first[i] << '\n';
        } else {
            out << i << '\t' << "NO" << '\n';
        }
    }
}

int main(int argc, char** argv) {
    int seed = 123;
    bool allow_no_aircraft = false;
    std::string input_filename = "../input_data/input.json";
    std::string data_filename = "../data/testing_data.json";
    std::shared_ptr<TemperatureFunction> temp_func;
    if (argc == 2) {
        auto args = GetArgs(argv[1]);
        seed = std::stoi(args[0]);
        if (std::stoi(args[1]) == 1) {
            allow_no_aircraft = true;
        }

        double initial_temperature = std::stod(args[3]);
        double alpha = std::stod(args[4]);

        if (args[2] == "log") {
            // std::cerr << "log function" << std::endl;
            temp_func = std::make_shared<LogarithmicTemperatureFunction>(initial_temperature, alpha);
        } else if (args[2] == "exp") {
            // std::cerr << "exp function" << std::endl;
            temp_func = std::make_shared<ExponentialTemperatureFunction>(initial_temperature, alpha);
        } else if (args[2] == "geom") {
            // std::cerr << "geom function" << std::endl;
            temp_func = std::make_shared<GeometricTemperatureFunction>(initial_temperature, alpha);
        } else {
            throw;
        }

        input_filename = args[5];
        data_filename = args[6];
    } else {
        throw;
    }

    std::mt19937 generator(seed);
    std::uniform_int_distribution<int> seed_dist(1, 1'000'000);

    auto reader = DataReader(input_filename, data_filename);
    auto input_data = reader.ReadData();

    std::pair<std::vector<int>, double> solution;
    SolutionCorrectnessInfo solution_status(true, true, true, true);
    for (int i = 0; i < constants::LAUNCHES_NUMBER; ++i) {
        auto simulated_annealing_solver =
            SimulatedAnnealingSolver(input_data,
                                     constants::ITERATIONS_NUMBER,
                                     seed_dist(generator),
                                     allow_no_aircraft,
                                     temp_func);
        // std::vector<int> initial_solution = {3, 3, 1, 3, 3, 1, 0, 0, 4, 4, 2, 2};
        // auto solution = simulated_annealing_solver.Solve(initial_solution);
        auto new_solution = simulated_annealing_solver.Solve();
        auto new_solution_status = simulated_annealing_solver.GetCorrectnessInfo();

        if (i == 0) {
            solution = new_solution;
            solution_status = new_solution_status;
        }

        if (new_solution_status.IsCorrect()) {
            if (!solution_status.IsCorrect() || new_solution.second < solution.second) {
                solution = new_solution;
                solution_status = new_solution_status;
            }
        }
    }
    PrintSolution(solution, solution_status, input_data->aircrafts->size(), std::cout);
    std::cerr << solution.second << std::endl;
    return 0;
}
