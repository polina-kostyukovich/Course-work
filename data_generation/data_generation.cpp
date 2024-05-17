#include <fstream>
#include <iostream>
#include <random>
#include <vector>

#include <Poco/JSON/JSON.h>
#include <Poco/JSON/Parser.h>

#include "../help_structs/entities.h"

using namespace Poco::JSON;

std::mt19937 generator;

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

bool TriangleInequalityOk(int side1, int side2, int side3) {
    return (side1 + side2 >= side3) && (side1 + side3 >= side2) && (side2 + side3 >= side1);
}

bool TriangleInequalitiesOk(const std::vector<std::vector<int>>& distances, int row, int column) {
    for (int i = 0; i < column; ++i) {
        if (!TriangleInequalityOk(distances[row][column], distances[row][i], distances[i][column])) {
            return false;
        }
    }
    return true;
}

std::vector<std::vector<int>> GetDistances(int airports_number,
                                           int min_distance,
                                           int max_distance) {
    std::vector<std::vector<int>> distances(airports_number, std::vector<int>(airports_number, 0));
    std::uniform_int_distribution<int> distance_dist(min_distance, max_distance);
    for (int i = 0; i < airports_number; ++i) {
        for (int j = 0; j < i; ++j) {
            distances[i][j] = distance_dist(generator);
            while (!TriangleInequalitiesOk(distances, i, j)) {
                distances[i][j] = distance_dist(generator);
            }
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
            std::uniform_int_distribution<int> departure_time_dist(1, hours_in_cycle - flight_time - 1);
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
                while (!(flight2.arrival_time + 1 <= flight1.departure_time - 1 ||
                        flight2.departure_time - 1 >= flight1.arrival_time + 1)) {
                    flight2.departure_time = departure_time_dist(generator);
                    flight2.arrival_time = flight2.departure_time + flight_time;
                }
                flight2.distance = distances[i][j];
                flight2.min_passengers = flight1.min_passengers;

                flights.push_back(flight1);
                flights.push_back(flight2);
            }
        }
    }
    return flights;
}

std::vector<Fleet> GenerateFleets(int fleets_number, int aircrafts_number,
                                  int min_passengers, int max_passengers,
                                  int min_cost, int max_cost) {
    std::uniform_int_distribution<int> seats_dist(min_passengers, max_passengers);
    std::uniform_int_distribution<int> cost_dist(min_cost, max_cost);
    std::vector<Fleet> fleets(fleets_number);

    for (int i = 0; i < fleets.size(); ++i) {
        std::uniform_int_distribution<int> aircrafts_number_dist(0, aircrafts_number);
        fleets[i].id = i;
        if (i < fleets.size() - 1) {
            fleets[i].aircrafts_number = aircrafts_number_dist(generator);
            aircrafts_number -= fleets[i].aircrafts_number;
        } else {
            fleets[i].aircrafts_number = aircrafts_number;
        }
        fleets[i].seats = seats_dist(generator);
        fleets[i].flight_cost = cost_dist(generator);
    }
    return fleets;
}

std::vector<Aircraft> GenerateAircrafts(const std::vector<Fleet>& fleets, int aircrafts_number) {
    std::vector<Aircraft> aircrafts;
    aircrafts.reserve(aircrafts_number);
    int id = 0;

    for (const auto& fleet : fleets) {
        for (int i = 0; i < fleet.aircrafts_number; ++i) {
            aircrafts.emplace_back(id, fleet.seats, fleet.flight_cost);
            id += 1;
        }
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

int main(int argc, char** argv) {
    int seed = 159807767;
    std::string input_filename = "../input_data/input.json";
    std::string output_filename = "../data/test4_14.json";
    if (argc == 2) {
        auto args = GetArgs(argv[1]);
        seed = std::stoi(args[0]);
        input_filename = args[1];
        output_filename = args[2];
    }
    generator.seed(seed);

    std::ifstream read_input_data(input_filename);
    std::string json_string, string;
    while (std::getline(read_input_data, string)) {
        json_string += string + '\n';
    }

    Parser parser;
    Object::Ptr pObject = parser.parse(json_string).extract<Object::Ptr>();
    int airports_number = pObject->getValue<int>("airports number");
    int flights_number = pObject->getValue<int>("flights number");
    int fleets_number = pObject->getValue<int>("fleets number");
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
                                   aircraft_speed, flights_number, min_passengers, max_passengers - 400);
    auto fleets = GenerateFleets(fleets_number, aircrafts_number, min_passengers + 400,
                                 max_passengers, min_flight_cost, max_flight_cost);
    auto aircrafts = GenerateAircrafts(fleets, aircrafts_number);
    auto airports = GenerateAirports(airports_number, min_stay_cost, max_stay_cost);

    auto flights_json = ArrayToJsonString(flights);
    auto aircrafts_json = ArrayToJsonString(aircrafts);
    auto airports_json = ArrayToJsonString(airports);
    std::string output_json = "{\n";
    output_json += "  \"flights\": " + flights_json + ",\n";
    output_json += "  \"aircrafts\": " + aircrafts_json + ",\n";
    output_json += "  \"airports\": " + airports_json + "\n";
    output_json += "}";
    std::ofstream write(output_filename);
    write << output_json << '\n';
    write.flush();
    return 0;
}
