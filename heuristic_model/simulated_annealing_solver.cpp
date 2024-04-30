#include "simulated_annealing_solver.h"

#include "../constants/heuristic_constants.h"
#include "../help_structs/data_processor.h"

#include <cmath>

SimulatedAnnealingSolver::SimulatedAnnealingSolver(const std::unique_ptr<InputData>& input_data,
                                     int iterations_number,
                                     int seed,
                                     bool allow_no_aircraft,
                                     const std::shared_ptr<TemperatureFunction>& temperature_function)
    : flights_(input_data->flights),
      generator_(seed),
      allow_no_aircraft_(allow_no_aircraft),
      aircrafts_number_(input_data->aircrafts->size()),
      iterations_number_(iterations_number),
      temperature_function_(temperature_function) {
    auto time_points = heuristic::GetTimePointsArrayForHeuristic(input_data->flights);
    heuristic::ReplaceTimePointsWithIndices(input_data->flights, time_points);
    model_ = HeuristicModel(input_data->aircrafts, input_data->airports, input_data->hours_in_cycle, time_points);
}

std::pair<std::vector<int>, double> SimulatedAnnealingSolver::Solve(const std::vector<int>& initial_solution) {
    solution_.resize(flights_->size());
    std::uniform_int_distribution<int> aircraft_dist(0, aircrafts_number_ - 1);
    std::uniform_int_distribution<int> index_dist(0, flights_->size() - 1);
    std::uniform_real_distribution<double> ksi_dist(0, 1);

    if (!initial_solution.empty()) {
        solution_ = initial_solution;
    } else {
        for (auto& item : solution_) {  // start only with real aircrafts
            item = aircraft_dist(generator_);
        }
    }
    for (int i = 0; i < solution_.size(); ++i) {
        model_.SetAircraftToFlight(solution_[i], (*flights_)[i]);
    }

    if (allow_no_aircraft_) {
        aircraft_dist = std::uniform_int_distribution<int>(0, aircrafts_number_);
    }

    for (int i = 0; i < iterations_number_; ++i) {
        auto prev_state = model_.GetTotalFine();
        auto index = index_dist(generator_);
        auto prev_aircraft = solution_[index];
        auto new_aircraft = aircraft_dist(generator_);
        if (new_aircraft == prev_aircraft) continue;

        double additional_fine = 0;
        if (prev_aircraft < aircrafts_number_) {
            model_.RemoveAircraftFromFlight(prev_aircraft, (*flights_)[index]);
        } else {
            additional_fine -= constants::NO_AIRCRAFT_FINE;
        }
        if (new_aircraft < aircrafts_number_) {  // real aircraft
            model_.SetAircraftToFlight(new_aircraft, (*flights_)[index]);
        } else {  // no aircraft
            additional_fine += constants::NO_AIRCRAFT_FINE;
        }
        auto new_state = model_.GetTotalFine();

        auto prev_fine = prev_state.fine + no_aircraft_fine_;
        auto new_fine = new_state.fine + additional_fine;

        auto ksi = ksi_dist(generator_);
        auto rhs = std::exp(-(new_fine - prev_fine) / (*temperature_function_)(i));

        if (new_fine < prev_fine + constants::EPSILON || ksi <= rhs) {  // change solution to a better one
            solution_[index] = new_aircraft;
            no_aircraft_fine_ += additional_fine;
            correctness_info_ = new_state.correctness_info;
        } else {
            correctness_info_ = prev_state.correctness_info;
            if (new_aircraft < aircrafts_number_) {  // real new aircraft
                model_.RemoveAircraftFromFlight(new_aircraft, (*flights_)[index]);
            }
            if (prev_aircraft < aircrafts_number_) {  // real previous aircraft
                model_.SetAircraftToFlight(prev_aircraft, (*flights_)[index]);
            }
        }
    }
    if (no_aircraft_fine_ > constants::EPSILON) {
        correctness_info_.has_no_aircraft = true;
    }
    return {solution_, model_.GetTotalFine().fine + no_aircraft_fine_};
}

SolutionCorrectnessInfo SimulatedAnnealingSolver::GetCorrectnessInfo() const {
    return correctness_info_;
}
