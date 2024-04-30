#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>

struct Flight {
    int id;
    int departure_airport;
    int arrival_airport;
    int departure_time;
    int arrival_time;
    int distance;
    int min_passengers;

    [[nodiscard]] std::string ToJsonString() const;
};

struct Aircraft {
    int id;
    int seats;
    int flight_cost;

    [[nodiscard]] std::string ToJsonString() const;
};

struct Fleet {
    int id;
    int aircrafts_number;
    int seats;
    int flight_cost;

    [[nodiscard]] std::string ToJsonString() const;
};

struct Airport {
    int id;
    int stay_cost;

    [[nodiscard]] std::string ToJsonString() const;
};

struct BaseInputData {
    int flights_number;
    int aircrafts_number;
};

struct InputData {
    std::shared_ptr<std::vector<Flight>> flights;
    std::shared_ptr<std::vector<Aircraft>> aircrafts;
    std::shared_ptr<std::vector<Airport>> airports;
    int hours_in_cycle;
};

struct SolutionCorrectnessInfo {
    bool flights_intersect;
    bool airports_mismatch;
    bool seats_lack;
    bool has_no_aircraft{false};

    bool IsCorrect() const {
        return !flights_intersect && !airports_mismatch && !seats_lack && !has_no_aircraft;
    }
};
