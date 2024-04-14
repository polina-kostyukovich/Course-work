#pragma once

#include <string>

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
