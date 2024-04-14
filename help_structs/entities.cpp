#include "entities.h"

std::string Flight::ToJsonString() const {
    std::string json = std::string("  {\n")
        + "      \"id\": " + std::to_string(id) + ",\n"
        + "      \"departure airport\": " + std::to_string(departure_airport) + ",\n"
        + "      \"arrival airport\": " + std::to_string(arrival_airport) + ",\n"
        + "      \"departure time\": " + std::to_string(departure_time) + ",\n"
        + "      \"arrival time\": " + std::to_string(arrival_time) + ",\n"
        + "      \"distance\": " + std::to_string(distance) + ",\n"
        + "      \"min passengers\": " + std::to_string(min_passengers) + "\n"
        + "    }";
    return json;
}

std::string Fleet::ToJsonString() const {
    std::string json = std::string("  {\n")
        + "      \"id\": " + std::to_string(id) + ",\n"
        + "      \"aircrafts number\": " + std::to_string(aircrafts_number) + ",\n"
        + "      \"seats\": " + std::to_string(seats) + ",\n"
        + "      \"flight cost\": " + std::to_string(flight_cost) + "\n"
        + "    }";
    return json;
}

std::string Aircraft::ToJsonString() const {
    std::string json = std::string("  {\n")
        + "      \"id\": " + std::to_string(id) + ",\n"
        + "      \"seats\": " + std::to_string(seats) + ",\n"
        + "      \"flight cost\": " + std::to_string(flight_cost) + "\n"
        + "    }";
    return json;
}

std::string Airport::ToJsonString() const {
    std::string json = std::string("  {\n")
        + "      \"id\": " + std::to_string(id) + ",\n"
        + "      \"stay cost\": " + std::to_string(stay_cost) + "\n"
        + "    }";
    return json;
}
