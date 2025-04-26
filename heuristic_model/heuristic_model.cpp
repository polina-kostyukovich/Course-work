#include "heuristic_model.h"

#include <numeric>
#include <iostream>

#include "../constants/heuristic_constants.h"

HeuristicModel::HeuristicModel(const std::shared_ptr<std::vector<Aircraft>>& aircrafts,
                               const std::shared_ptr<std::vector<Airport>>& airports,
                               int hours_in_cycle,
                               int hour_size,
                               const std::vector<int>& time_points)
    : aircrafts_(aircrafts),
      flights_at_times_(FlightsAtTimes(aircrafts->size(), time_points.size())),
      end_points_(std::vector<AircraftEndPoints>(aircrafts->size(), AircraftEndPoints{airports, hours_in_cycle, hour_size, time_points})),
      flights_cost_(aircrafts) {}

void HeuristicModel::SetAircraftToFlight(int aircraft, const Flight& flight) {
    seats_fine_ += std::max(0, flight.min_passengers - (*aircrafts_)[aircraft].seats);
    flights_at_times_.SetAircraftToFlight(aircraft, flight);
    end_points_[aircraft].SetFlight(flight);
    flights_cost_.SetAircraftToFlight(aircraft, flight);
}

void HeuristicModel::RemoveAircraftFromFlight(int aircraft, const Flight& flight) {
    seats_fine_ -= std::max(0, flight.min_passengers - (*aircrafts_)[aircraft].seats);
    flights_at_times_.RemoveAircraftFromFlight(aircraft, flight);
    end_points_[aircraft].RemoveFlight(flight);
    flights_cost_.RemoveAircraftFromFlight(aircraft, flight);
}

HeuristicModel::ModelState HeuristicModel::GetTotalFine() const {
    int64_t airport_mismatch_fine = std::accumulate(end_points_.begin(),
                                                    end_points_.end(),
                                                    0,
                                                    [](int64_t acc, const AircraftEndPoints& value){
        return acc + value.GetTotalFine();
    });
    int64_t total_stay_cost = std::accumulate(end_points_.begin(),
                                              end_points_.end(),
                                              0,
                                              [](int64_t acc, const AircraftEndPoints& value){
        return acc + value.GetStayCost();
    });

    SolutionCorrectnessInfo correctness_info;
    correctness_info.flights_intersect = flights_at_times_.GetTotalFine() != 0;
    correctness_info.airports_mismatch = airport_mismatch_fine != 0;
    correctness_info.seats_lack = seats_fine_ != 0;
    auto total_fine = flights_at_times_.GetTotalFine() * constants::FLIGHTS_INTERSECTION_FINE
            + airport_mismatch_fine * constants::AIRPORTS_MISMATCH_FINE
            + seats_fine_ * constants::SEATS_FINE
            + flights_cost_.GetTotalFine() * constants::FLIGHTS_COST
            + total_stay_cost * constants::STAY_COST;
    return {total_fine, correctness_info};
}

HeuristicModel::FlightsAtTimes::FlightsAtTimes(int aircrafts_number, int time_points_number)
    : flights_number_(std::vector<std::vector<int>>(
        aircrafts_number,std::vector<int>(time_points_number))) {}

void HeuristicModel::FlightsAtTimes::SetAircraftToFlight(int aircraft, const Flight& flight) {
    for (int time_point = flight.departure_time; time_point <= flight.arrival_time; ++time_point) {
        if (flights_number_[aircraft][time_point] >= 1) {
            fine_ += 1;
        }
        flights_number_[aircraft][time_point] += 1;
    }
}

void HeuristicModel::FlightsAtTimes::RemoveAircraftFromFlight(int aircraft, const Flight& flight) {
    for (int time_point = flight.departure_time; time_point <= flight.arrival_time; ++time_point) {
        if (flights_number_[aircraft][time_point] > 1) {
            fine_ -= 1;
        }
        flights_number_[aircraft][time_point] -= 1;
    }
}

int HeuristicModel::FlightsAtTimes::GetTotalFine() const {
    return fine_;
}

bool HeuristicModel::AircraftEndPoints::EndPoint::operator<(
    const HeuristicModel::AircraftEndPoints::EndPoint& other) const {
    if (time_point == other.time_point) {
        if (stage == other.stage) {
            return airport < other.airport;
        }
        return stage < other.stage;
    }
    return time_point < other.time_point;
}

void HeuristicModel::AircraftEndPoints::SetFlight(const Flight& flight) {
    EndPoint departure_point({flight.departure_time, flight.departure_airport, FlightStage::kDeparture});
    EndPoint arrival_point({flight.arrival_time, flight.arrival_airport, FlightStage::kArrival});
    InsertTimePoint(departure_point);
    InsertTimePoint(arrival_point);
}

void HeuristicModel::AircraftEndPoints::RemoveFlight(const Flight& flight) {
    auto departure = end_points_.find({flight.departure_time, flight.departure_airport, FlightStage::kDeparture});
    RemoveTimePoint(departure);
    auto arrival = end_points_.find({flight.arrival_time, flight.arrival_airport, FlightStage::kArrival});
    RemoveTimePoint(arrival);
}

int HeuristicModel::AircraftEndPoints::GetTotalFine() const {
    return mismatches_number_;
}

bool HeuristicModel::AircraftEndPoints::HasStay(
    std::multimap<HeuristicModel::AircraftEndPoints::EndPoint, int>::iterator left,
    std::multimap<HeuristicModel::AircraftEndPoints::EndPoint, int>::iterator right) {
    return left->first.stage == FlightStage::kArrival &&
            right->first.stage == FlightStage::kDeparture;
}

bool HeuristicModel::AircraftEndPoints::HasConflict(
    std::multimap<HeuristicModel::AircraftEndPoints::EndPoint, int>::iterator left,
    std::multimap<HeuristicModel::AircraftEndPoints::EndPoint, int>::iterator right) {
    return HasStay(left, right) && left->first.airport != right->first.airport;
}

std::multimap<HeuristicModel::AircraftEndPoints::EndPoint, int>::iterator HeuristicModel::AircraftEndPoints::GetPrev(
    std::multimap<HeuristicModel::AircraftEndPoints::EndPoint, int>::iterator it) {
    if (it == end_points_.begin()) {
        it = end_points_.end();
        --it;
    } else {
        --it;
    }
    return it;
}

std::multimap<HeuristicModel::AircraftEndPoints::EndPoint, int>::iterator HeuristicModel::AircraftEndPoints::GetNext(
    std::multimap<HeuristicModel::AircraftEndPoints::EndPoint, int>::iterator it) {
    ++it;
    if (it == end_points_.end()) {
        it = end_points_.begin();
    }
    return it;
}

void HeuristicModel::AircraftEndPoints::InsertTimePoint(
        const HeuristicModel::AircraftEndPoints::EndPoint& point) {
    auto it = end_points_.insert({point, 0});
    if (end_points_.size() == 1) {
        return;
    }
    if (end_points_.size() == 2) {
        auto first_element = end_points_.begin();
        auto second_element = ++end_points_.begin();
        if (first_element->first.stage == second_element->first.stage) return;

        if (first_element->first.stage == FlightStage::kArrival) {
            first_element->second = 1;
        } else {
            second_element->second = 1;
        }
        mismatches_number_ += 1;

        if (HasStay(first_element, second_element)) {
            stay_cost_ += GetStayTime(first_element, second_element) * (*airports_)[first_element->first.airport].stay_cost;
        } else if (HasStay(second_element, first_element)) {
            stay_cost_ += GetStayTime(second_element, first_element) * (*airports_)[second_element->first.airport].stay_cost;
        }
        return;
    }

    auto prev = GetPrev(it);
    auto next = GetNext(it);

    if (HasStay(prev, next)) {
        mismatches_number_ -= prev->second;  // remove conflict if was
        prev->second = 0;

        stay_cost_ -= GetStayTime(prev, next) * (*airports_)[prev->first.airport].stay_cost;
    }

    // check new conflicts
    if (HasConflict(prev, it)) {
        prev->second = 1;
        mismatches_number_ += 1;
    } else if (HasConflict(it, next)) {
        it->second = 1;
        mismatches_number_ += 1;
    }

    if (HasStay(prev, it)) {
        stay_cost_ += GetStayTime(prev, it) * (*airports_)[prev->first.airport].stay_cost;
    }
    if (HasStay(it, next)) {
        stay_cost_ += GetStayTime(it, next) * (*airports_)[it->first.airport].stay_cost;
    }
}

void HeuristicModel::AircraftEndPoints::RemoveTimePoint(
        std::multimap<HeuristicModel::AircraftEndPoints::EndPoint, int>::iterator point) {
    if (end_points_.empty()) throw;
    if (end_points_.size() == 1) {
        end_points_.erase(point);
        stay_cost_ = 0;
        return;
    }

    auto prev = GetPrev(point);
    auto next = GetNext(point);

    // remove conflict if was
    if (HasStay(point, next)) {
        mismatches_number_ -= point->second;
        stay_cost_ -= GetStayTime(point, next) * (*airports_)[point->first.airport].stay_cost;
    } else if (HasStay(prev, point)) {
        mismatches_number_ -= prev->second;
        prev->second = 0;
        stay_cost_ -= GetStayTime(prev, point) * (*airports_)[prev->first.airport].stay_cost;
    }

    end_points_.erase(point);
    if (HasConflict(prev, next)) {
        prev->second = 1;
        mismatches_number_ += 1;
    }
    if (HasStay(prev, next)) {
        stay_cost_ += GetStayTime(prev, next) * (*airports_)[prev->first.airport].stay_cost;
    }
}

int HeuristicModel::AircraftEndPoints::GetStayCost() const {
    return stay_cost_;
}

int HeuristicModel::AircraftEndPoints::GetStayTime(
        std::multimap<HeuristicModel::AircraftEndPoints::EndPoint, int>::iterator left,
        std::multimap<HeuristicModel::AircraftEndPoints::EndPoint, int>::iterator right) {
    int stay_time = time_points_[right->first.time_point] - time_points_[left->first.time_point];
    if (stay_time < 0) {
        stay_time += hours_in_cycle_;
    }
    return stay_time + 2 * hour_size_;
}

HeuristicModel::FlightsCost::FlightsCost(const std::shared_ptr<std::vector<Aircraft>>& aircrafts)
    : aircrafts_(aircrafts) {}

void HeuristicModel::FlightsCost::SetAircraftToFlight(int aircraft, const Flight& flight) {
    int diff = (*aircrafts_)[aircraft].flight_cost * flight.distance;
    fine_ += diff;
}

void HeuristicModel::FlightsCost::RemoveAircraftFromFlight(int aircraft, const Flight& flight) {
    int diff = (*aircrafts_)[aircraft].flight_cost * flight.distance;
    fine_ -= diff;
}

int HeuristicModel::FlightsCost::GetTotalFine() const {
    return fine_;
}
