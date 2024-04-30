#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include "constants/heuristic_constants.h"
#include "help_structs/data_reader.h"
#include "help_structs/entities.h"
#include "heuristic_model/local_search_solver.h"

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

int main() {
    const int kSeed = 42569;
    const std::string input_filename = "../input_data/input.json";
    const std::string data_filename = "../data/testing_data.json";
    auto reader = DataReader(input_filename, data_filename);
    auto input_data = reader.ReadData();

    auto local_search_solver =
        LocalSearchSolver(input_data, constants::ITERATIONS_NUMBER, kSeed, true);
    // std::vector<int> initial_solution = {3, 3, 1, 3, 3, 1, 0, 0, 4, 4, 2, 2};
    // auto solution = local_search_solver.Solve(initial_solution);
    auto solution = local_search_solver.Solve();
    auto solution_status = local_search_solver.GetCorrectnessInfo();
    PrintSolution(solution, solution_status, input_data->aircrafts->size(), std::cout);
    return 0;
}
