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

    std::vector<int> Solve();

private:
    static double NO_AIRCRAFT_FINE;

private:
    std::mt19937 generator_;
    HeuristicModel model_;
    std::shared_ptr<std::vector<Flight>> flights_;
    std::vector<int> solution_;
    bool allow_no_aircraft_;
    int aircrafts_number_;
    int iterations_number_;
};
