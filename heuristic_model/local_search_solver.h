#pragma once

#include <memory>
#include <random>
#include <vector>

#include "heuristic_model.h"
#include "../help_structs/entities.h"

class LocalSearchSolver {
public:
    explicit LocalSearchSolver(const std::unique_ptr<InputData>& input_data,
                               int iterations_number,
                               int seed,
                               bool allow_no_aircraft);

    std::pair<std::vector<int>, double> Solve(const std::vector<int>& initial_solution = {});

    SolutionCorrectnessInfo GetCorrectnessInfo() const;

private:
    std::mt19937 generator_;
    HeuristicModel model_;
    std::shared_ptr<std::vector<Flight>> flights_;
    std::vector<int> solution_;
    bool allow_no_aircraft_;
    int aircrafts_number_;
    int iterations_number_;
    double no_aircraft_fine_{0};
    SolutionCorrectnessInfo correctness_info_;
};
