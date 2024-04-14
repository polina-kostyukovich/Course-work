#include "heuristic_model.h"

#include <numeric>

double HeuristicModel::AIRPORTS_MISMATCH_FINE = 1e6;
double HeuristicModel::FLIGHTS_INTERSECTION_FINE = 1e3;

HeuristicModel::HeuristicModel(const std::shared_ptr<std::vector<Aircraft>>& aircrafts)
    : aircrafts_(aircrafts) {}

void HeuristicModel::SetAircraftToFlight(int aircraft, const Flight& flight) {
    seats_fine_ += std::max(0, flight.min_passengers - (*aircrafts_)[aircraft].seats);

    flights_at_times_.SetAircraftToFlight(aircraft, flight);
    end_points_[aircraft].SetFlight(flight);
}

void HeuristicModel::RemoveAircraftFromFlight(int aircraft, const Flight& flight) {
    seats_fine_ -= std::max(0, flight.min_passengers - (*aircrafts_)[aircraft].seats);

    flights_at_times_.RemoveAircraftFromFlight(aircraft, flight);
    end_points_[aircraft].RemoveFlight(flight);
}

double HeuristicModel::GetTotalFine() const {
    int64_t airport_mismatch = std::accumulate(end_points_.begin(),
                                               end_points_.end(),
                                               0,
                                               [](int64_t acc, const AircraftEndPoints& value){
      return acc + value.GetTotalFine();
    });
    return AIRPORTS_MISMATCH_FINE * airport_mismatch
            + FLIGHTS_INTERSECTION_FINE * flights_at_times_.GetTotalFine()
            + seats_fine_;
}

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


bool HeuristicModel::AircraftEndPoints::EndPoint::operator<(const HeuristicModel::AircraftEndPoints::EndPoint& other) const {
    if (time_point != other.time_point) {
        return time_point < other.time_point;
    }
    return stage < other.stage;
}

void HeuristicModel::AircraftEndPoints::SetFlight(const Flight& flight) {
    EndPoint departure_point({flight.departure_time, FlightStage::kDeparture});
    EndPoint arrival_point({flight.arrival_time, FlightStage::kArrival});
    InsertTimePoint(departure_point, flight.departure_airport);
    InsertTimePoint(arrival_point, flight.arrival_airport);
}

void HeuristicModel::AircraftEndPoints::RemoveFlight(const Flight& flight) {
    auto departure = end_points_.find({flight.departure_time, FlightStage::kDeparture});
    auto arrival = end_points_.find({flight.arrival_time, FlightStage::kArrival});
    RemoveTimePoint(departure);
    RemoveTimePoint(arrival);
}

int HeuristicModel::AircraftEndPoints::GetTotalFine() const {
    return mismatches_number_;
}

bool HeuristicModel::AircraftEndPoints::HasConflict(
    std::multimap<HeuristicModel::AircraftEndPoints::EndPoint,
                  HeuristicModel::AircraftEndPoints::AirportAndFine>::iterator left,
    std::multimap<HeuristicModel::AircraftEndPoints::EndPoint,
                  HeuristicModel::AircraftEndPoints::AirportAndFine>::iterator right) {
    return left->first.stage == FlightStage::kArrival &&
            right->first.stage == FlightStage::kDeparture &&
            left->second.airport != right->second.airport;
}

std::multimap<HeuristicModel::AircraftEndPoints::EndPoint,
              HeuristicModel::AircraftEndPoints::AirportAndFine>::iterator HeuristicModel::AircraftEndPoints::GetPrev(
    std::multimap<HeuristicModel::AircraftEndPoints::EndPoint,
                  HeuristicModel::AircraftEndPoints::AirportAndFine>::iterator it) {
    if (it == end_points_.begin()) {
        it = --end_points_.end();
    } else {
        --it;
    }
    return it;
}

std::multimap<HeuristicModel::AircraftEndPoints::EndPoint,
              HeuristicModel::AircraftEndPoints::AirportAndFine>::iterator HeuristicModel::AircraftEndPoints::GetNext(
    std::multimap<HeuristicModel::AircraftEndPoints::EndPoint,
                  HeuristicModel::AircraftEndPoints::AirportAndFine>::iterator it) {
    ++it;
    if (it == end_points_.end()) {
        it = end_points_.begin();
    }
    return it;
}

void HeuristicModel::AircraftEndPoints::InsertTimePoint(
        const HeuristicModel::AircraftEndPoints::EndPoint& point, int airport) {
    auto it = end_points_.insert({point, {airport, 0}});
    if (end_points_.size() == 1) return;
    if (end_points_.size() == 2) {
        auto first_element = end_points_.begin();
        auto second_element = ++end_points_.begin();
        if (first_element->first.stage == second_element->first.stage) return;

        if (first_element->first.stage == FlightStage::kArrival) {
            first_element->second.fine = 1;
        } else {
            second_element->second.fine = 1;
        }
        mismatches_number_ += 1;
    }

    auto prev = GetPrev(it);
    auto next = GetNext(it);

    if (prev->first.stage == FlightStage::kArrival && next->first.stage == FlightStage::kDeparture) {
        mismatches_number_ -= prev->second.fine;  // remove conflict if was
        prev->second.fine = 0;
    }

    // check new conflicts
    if (HasConflict(prev, it)) {
        prev->second.fine = 1;
        mismatches_number_ += 1;
    } else if (HasConflict(it, next)) {
        it->second.fine = 1;
        mismatches_number_ += 1;
    }
}

void HeuristicModel::AircraftEndPoints::RemoveTimePoint(
        std::multimap<HeuristicModel::AircraftEndPoints::EndPoint,
                      HeuristicModel::AircraftEndPoints::AirportAndFine>::iterator point) {
    if (end_points_.size() == 1) {
        end_points_.erase(point);
        return;
    }

    auto prev = GetPrev(point);
    auto next = GetNext(point);
    // remove conflict if was
    if (point->first.stage == FlightStage::kArrival) {
        mismatches_number_ -= point->second.fine;
    } else {
        mismatches_number_ -= prev->second.fine;
        prev->second.fine = 0;
    }

    end_points_.erase(point);
    if (HasConflict(prev, next)) {
        prev->second.fine = 1;
        mismatches_number_ += 1;
    }
}
