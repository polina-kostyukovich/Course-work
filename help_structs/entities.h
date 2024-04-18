#pragma once

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

struct InputData {
    std::shared_ptr<std::vector<Flight>> flights;
    std::shared_ptr<std::vector<Aircraft>> aircrafts;
    std::shared_ptr<std::vector<Airport>> airports;
    int hours_in_cycle;
};
