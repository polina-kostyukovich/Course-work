#include <fstream>
#include <iostream>
#include <random>
#include <vector>

#include <Poco/JSON/JSON.h>
#include <Poco/JSON/Parser.h>

using namespace Poco::JSON;

std::mt19937 generator(2836);

struct Flight {
    int id;
    int departure_airport;
    int arrival_airport;
    int departure_time;
    int arrival_time;
    int distance;
    int min_passengers;

    [[nodiscard]] std::string ToJsonString() const {
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
};

struct Aircraft {
    int id;
    int seats;
    int flight_cost;

    [[nodiscard]] std::string ToJsonString() const {
        std::string json = std::string("  {\n")
            + "      \"id\": " + std::to_string(id) + ",\n"
            + "      \"seats\": " + std::to_string(seats) + ",\n"
            + "      \"flight cost\": " + std::to_string(flight_cost) + "\n"
            + "    }";
        return json;
    }
};

struct Airport {
    int id;
    int stay_cost;

    [[nodiscard]] std::string ToJsonString() const {
        std::string json = std::string("  {\n")
            + "      \"id\": " + std::to_string(id) + ",\n"
            + "      \"stay cost\": " + std::to_string(stay_cost) + "\n"
            + "    }";
        return json;
    }
};

template <typename T>
std::string ArrayToJsonString(const std::vector<T>& array) {
    std::string json = "[\n";
    for (const auto& item : array) {
        json += "  " + item.ToJsonString() + ",\n";
    }
    json.pop_back();
    json.pop_back();
    json += "\n  ]";
    return json;
}

std::vector<std::vector<int>> GetDistances(int airports_number,
                                           int min_distance,
                                           int max_distance) {
    std::vector<std::vector<int>> distances(airports_number, std::vector<int>(airports_number));
    // std::normal_distribution<double>
    //     distance_dist(static_cast<double>(min_distance + max_distance) / 2,
    //                   static_cast<double>(max_distance - min_distance) / 2);
    std::uniform_int_distribution<int> distance_dist(min_distance, max_distance);
    for (int i = 0; i < airports_number; ++i) {
        for (int j = 0; j < i; ++j) {
            if (i == j) {
                distances[i][j] = 0;
                continue;
            }
            distances[i][j] = static_cast<int>(distance_dist(generator));
            distances[j][i] = distances[i][j];
        }
    }
    return distances;
}

std::vector<Flight> GenerateFlights(const std::vector<std::vector<int>>& graph, int hours_in_cycle,
                                    int min_distance, int max_distance, int aircraft_speed,
                                    int flights_number, int min_passengers, int max_passengers) {
    auto distances = GetDistances(graph.size(), min_distance, max_distance);

    std::uniform_int_distribution<int> passengers_dist(min_passengers, max_passengers);
    std::vector<Flight> flights;
    flights.reserve(flights_number);
    for (int i = 0; i < graph.size(); ++i) {
        for (int j = i + 1; j < graph.size(); ++j) {
            int flight_time = static_cast<int>(std::ceil(
                static_cast<double>(distances[i][j]) / aircraft_speed));
            std::uniform_int_distribution<int> departure_time_dist(0, hours_in_cycle - flight_time);
            for (int k = 0; k < graph[i][j]; ++k) {
                Flight flight1;
                flight1.id = flights.size();
                flight1.departure_airport = i;
                flight1.arrival_airport = j;
                flight1.departure_time = departure_time_dist(generator);
                flight1.arrival_time = flight1.departure_time + flight_time;
                flight1.distance = distances[i][j];
                flight1.min_passengers = passengers_dist(generator);

                Flight flight2;
                flight2.id = flights.size() + 1;
                flight2.departure_airport = j;
                flight2.arrival_airport = i;
                flight2.departure_time = departure_time_dist(generator);
                flight2.arrival_time = flight2.departure_time + flight_time;
                flight2.distance = distances[i][j];
                flight2.min_passengers = flight1.min_passengers;

                flights.push_back(flight1);
                flights.push_back(flight2);
            }
        }
    }
    return flights;
}

std::vector<Aircraft> GenerateAircrafts(int aircrafts_number, int min_passengers, int max_passengers,
                                        int min_cost, int max_cost) {
    std::uniform_int_distribution<int> seats_dist(min_passengers, max_passengers);
    std::uniform_int_distribution<int> cost_dist(min_cost, max_cost);
    std::vector<Aircraft> aircrafts(aircrafts_number);

    for (int i = 0; i < aircrafts.size(); ++i) {
        aircrafts[i].id = i;
        aircrafts[i].seats = seats_dist(generator);
        aircrafts[i].flight_cost = cost_dist(generator);
    }
    return aircrafts;
}

std::vector<Airport> GenerateAirports(int airports_number, int min_stay_cost, int max_stay_cost) {
    std::uniform_int_distribution<int> cost_dist(min_stay_cost, max_stay_cost);
    std::vector<Airport> airports(airports_number);
    for (int i = 0; i < airports_number; ++i) {
        airports[i].id = i;
        airports[i].stay_cost = cost_dist(generator);
    }
    return airports;
}

int main() {
    std::ifstream read_input_data("../input_data/input.json");
    std::string json_string, string;
    while (std::getline(read_input_data, string)) {
        json_string += string + '\n';
    }

    Parser parser;
    Object::Ptr pObject = parser.parse(json_string).extract<Object::Ptr>();
    int airports_number = pObject->getValue<int>("airports number");
    int flights_number = pObject->getValue<int>("flights number");
    int hours_in_cycle = pObject->getValue<int>("hours in cycle");
    int min_distance = pObject->getValue<int>("min distance");
    int max_distance = pObject->getValue<int>("max distance");
    int aircraft_speed = pObject->getValue<int>("aircraft speed");
    int aircrafts_number = pObject->getValue<int>("aircrafts number");
    int min_passengers = pObject->getValue<int>("min passengers number");
    int max_passengers = pObject->getValue<int>("max passengers number");
    int min_flight_cost = pObject->getValue<int>("min flight cost");
    int max_flight_cost = pObject->getValue<int>("max flight cost");
    int min_stay_cost = pObject->getValue<int>("min stay cost");
    int max_stay_cost = pObject->getValue<int>("max stay cost");

    std::ifstream read_graph("../data/graph.txt");
    std::vector<std::vector<int>> graph(airports_number, std::vector<int>(airports_number));
    for (auto& edges_list : graph) {
        for (auto& edges_number : edges_list) {
            read_graph >> edges_number;
        }
    }

    auto flights = GenerateFlights(graph, hours_in_cycle, min_distance, max_distance,
                                   aircraft_speed, flights_number, min_passengers, max_passengers);
    auto aircrafts = GenerateAircrafts(aircrafts_number, min_passengers, max_passengers,
                                       min_flight_cost, max_flight_cost);
    auto airports = GenerateAirports(airports_number, min_stay_cost, max_stay_cost);

    auto flights_json = ArrayToJsonString(flights);
    auto aircrafts_json = ArrayToJsonString(aircrafts);
    auto airports_json = ArrayToJsonString(airports);
    std::string output_json = "{\n";
    output_json += "  \"flights\": " + flights_json + ",\n";
    output_json += "  \"aircrafts\": " + aircrafts_json + ",\n";
    output_json += "  \"airports\": " + airports_json + "\n";
    output_json += "}";
    std::ofstream write("../data/testing_data.json");
    write << output_json << '\n';
    return 0;
}
