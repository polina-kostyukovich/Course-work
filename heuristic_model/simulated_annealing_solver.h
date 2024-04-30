#pragma once

#include <memory>
#include <random>
#include <vector>

#include "heuristic_model.h"
#include "../help_structs/entities.h"
#include "../help_structs/temperature_functions.h"

class SimulatedAnnealingSolver {
public:
    explicit SimulatedAnnealingSolver(const std::unique_ptr<InputData>& input_data,
                                      int iterations_number,
                                      int seed,
                                      bool allow_no_aircraft,
                                      const std::shared_ptr<TemperatureFunction>& temperature_function);

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
    std::shared_ptr<TemperatureFunction> temperature_function_;
};
