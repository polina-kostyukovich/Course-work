#include "local_search_solver.h"

#include "../constants/heuristic_constants.h"

LocalSearchSolver::LocalSearchSolver(const std::unique_ptr<InputData>& input_data,
                                     int iterations_number,
                                     int seed,
                                     bool allow_no_aircraft)
    : model_(input_data->aircrafts, input_data->airports, input_data->hours_in_cycle),
      flights_(input_data->flights),
      generator_(seed),
      allow_no_aircraft_(allow_no_aircraft),
      aircrafts_number_(input_data->aircrafts->size()),
      iterations_number_(iterations_number) {}

std::vector<int> LocalSearchSolver::Solve() {
    solution_.resize(flights_->size());
    std::uniform_int_distribution<int> aircraft_dist(0, aircrafts_number_ - 1);
    std::uniform_int_distribution<int> index_dist(0, flights_->size() - 1);

    for (int i = 0; i < solution_.size(); ++i) {
        solution_[i] = aircraft_dist(generator_);
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

        model_.RemoveAircraftFromFlight(prev_aircraft, (*flights_)[index]);
        double additional_fine = 0;
        if (new_aircraft < flights_->size()) {  // real aircraft
            model_.SetAircraftToFlight(new_aircraft, (*flights_)[index]);
        } else {  // no aircraft
            additional_fine += constants::NO_AIRCRAFT_FINE;
        }
        auto new_state = model_.GetTotalFine();
        new_state.fine += additional_fine;

        if (new_state.fine > prev_state.fine) {
            correctness_info_ = prev_state.correctness_info;
            if (additional_fine == 0) {  // real aircraft
                model_.RemoveAircraftFromFlight(new_aircraft, (*flights_)[index]);
            } else {  // fake aircraft
                correctness_info_ = {true, true, true};
            }
            model_.SetAircraftToFlight(prev_aircraft, (*flights_)[index]);
        } else {  // change solution
            solution_[index] = new_aircraft;
            correctness_info_ = new_state.correctness_info;
        }
    }
    return solution_;
}

SolutionCorrectnessInfo LocalSearchSolver::GetCorrectnessInfo() const {
    return correctness_info_;
}


