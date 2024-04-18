#include "local_search_solver.h"

double LocalSearchSolver::NO_AIRCRAFT_FINE = 1e3;

LocalSearchSolver::LocalSearchSolver(const std::unique_ptr<InputData>& input_data,
                                     int iterations_number,
                                     int seed,
                                     bool allow_no_aircraft)
    : model_(input_data->aircrafts, input_data->airports, input_data->hours_in_cycle),
      flights_(input_data->flights),
      generator_(seed),
      allow_no_aircraft_(allow_no_aircraft),
      aircrafts_number_(allow_no_aircraft ? input_data->aircrafts->size() + 1 : input_data->aircrafts->size()),
      iterations_number_(iterations_number) {}

std::vector<int> LocalSearchSolver::Solve() {
    solution_.resize(flights_->size());
    std::uniform_int_distribution<int> aircraft_dist(0, aircrafts_number_);
    std::uniform_int_distribution<int> index_dist(0, flights_->size() - 1);

    for (int i = 0; i < solution_.size(); ++i) {
        solution_[i] = aircraft_dist(generator_);
        model_.SetAircraftToFlight(solution_[i], (*flights_)[i]);
    }

    for (int i = 0; i < iterations_number_; ++i) {
        auto prev_fine = model_.GetTotalFine();
        auto index = index_dist(generator_);
        model_.RemoveAircraftFromFlight(solution_[index], (*flights_)[index]);
        auto new_value = aircraft_dist(generator_);
        double additional_fine = 0;
        if (new_value < flights_->size()) {
            model_.SetAircraftToFlight(new_value, (*flights_)[index]);
        } else {
            additional_fine += NO_AIRCRAFT_FINE;
        }
        auto new_fine = model_.GetTotalFine() + additional_fine;
        // todo
    }
    return solution_;
}


