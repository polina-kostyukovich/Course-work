#include <fstream>
#include <iostream>
#include <random>
#include <vector>

#include <Poco/JSON/JSON.h>
#include <Poco/JSON/Object.h>
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

std::vector<int> GetRandomArrayInRange(int array_size, int elements_sum, int min_number, int max_number) {
    assert(elements_sum >= array_size * min_number);
    assert(elements_sum <= array_size * max_number);
    assert(array_size % 2 != 0 || elements_sum % 2 == 0);

    std::vector<int> result(array_size);
    int current_sum = 0;
    for (int i = 0; i < array_size; i += 2) {
        if (i == array_size - 1) {
            result[i] = elements_sum - current_sum;
            break;
        }
        int current_max = std::min(
                static_cast<int>(std::floor((elements_sum - current_sum - (array_size - i - 2) * min_number) / 2)),
                max_number);
        int current_min = std::max(
                static_cast<int>(std::ceil((elements_sum - current_sum - (array_size - i - 2) * max_number) / 2)),
                min_number);
        std::uniform_int_distribution<int> new_dist(current_min, current_max);
        result[i + 1] = result[i] = new_dist(generator);
        current_sum += 2 * result[i];
    }
    return result;
}

// total flight time of single aircraft (with specified airports)
int GetTotalFlightTime(const std::vector<int>& airports,
                       const std::vector<std::vector<int>>& distances,
                       int aircraft_speed,
                       int hour_size) {
    int total_flight_time = 2 * hour_size * (static_cast<int>(airports.size()) - 1);  // additional 2 hours per flight
    for (int j = 0; j < static_cast<int>(airports.size()) - 1; ++j) {
        int from = airports[j];
        int to = airports[j + 1];
        int flight_time = static_cast<int>(std::ceil(static_cast<double>(distances[from][to]) / aircraft_speed));
        total_flight_time += flight_time;
    }
    return total_flight_time;
}

// generating flights for single aircraft
std::vector<Flight> GetAircraftFlights(const std::vector<int>& airports,
                                       const std::vector<std::vector<int>>& distances,
                                       int aircraft_speed,
                                       int hours_in_cycle,
                                       int hour_size,
                                       int min_passengers,
                                       int max_passengers) {
    const int flights_number = static_cast<int>(airports.size()) - 1;
    std::uniform_int_distribution<int> passengers_dist(min_passengers, max_passengers);
    int total_flight_time = GetTotalFlightTime(airports, distances, aircraft_speed, hour_size);
    std::vector<Flight> flights;
    flights.reserve(flights_number);
    for (int j = 0; j < flights_number; ++j) {
        int from = airports[j];
        int to = airports[j + 1];
        int flight_time = static_cast<int>(std::ceil(
                static_cast<double>(distances[from][to]) / aircraft_speed));
        int min_start_time = flights.empty() ? hour_size : (flights.back().arrival_time + 2 * hour_size);
        int max_start_time = hours_in_cycle - total_flight_time + hour_size;
        std::uniform_int_distribution<int> departure_time_dist(min_start_time, max_start_time);
        Flight flight = {
                .id = 0,
                .departure_airport = from,
                .arrival_airport = to,
                .departure_time = departure_time_dist(generator),
                .arrival_time = flight.departure_time + flight_time,
                .distance = distances[from][to],
                .min_passengers = passengers_dist(generator),
        };
        flights.push_back(flight);
        total_flight_time -= flight_time + 2 * hour_size;
    }
    return flights;
}

// flights + aircrafts_to_flights_number
std::pair<std::vector<Flight>, std::vector<int>> GenerateFlights(int airports_number,
                                                                int aircrafts_number,
                                                                int hours_in_cycle,
                                                                int hour_size,
                                                                int min_distance,
                                                                int max_distance,
                                                                int aircraft_speed,
                                                                int flights_number,
                                                                int min_passengers,
                                                                int max_passengers,
                                                                int min_flights_per_aircraft,
                                                                int max_flights_per_aircraft) {
    auto distances = GetDistances(airports_number, min_distance, max_distance);

    std::uniform_int_distribution<int> airport_dist(0, airports_number - 1);
    auto flights_per_aircraft = GetRandomArrayInRange(aircrafts_number, flights_number, min_flights_per_aircraft, max_flights_per_aircraft);
    std::vector<Flight> flights;
    flights.reserve(flights_number);
    for (int i = 0; i < flights_per_aircraft.size(); i += 2) {
        // if aircrafts_number % 2 == 1
        if (i == flights_per_aircraft.size() - 1) {
            flights_number = flights_per_aircraft.back() / 2;
            // generating airports
            std::vector<int> airports(2 * flights_number + 1);
            for (int j = 0; j <= flights_number; ++j) {
                airports[j] = airport_dist(generator);
                while (j > 0 && airports[j] == airports[j - 1]) {
                    airports[j] = airport_dist(generator);
                }
                airports[airports.size() - 1 - j] = airports[j];
            }
            auto cur_flights = GetAircraftFlights(
                    airports,
                    distances,
                    aircraft_speed,
                    hours_in_cycle,
                    hour_size,
                    min_passengers,
                    max_passengers
            );
            flights.append_range(cur_flights);
            break;
        }

        // generating airports
        std::vector<int> airports(flights_per_aircraft[i] + 1);
        for (int j = 0; j < flights_per_aircraft[i]; ++j) {
            airports[j] = airport_dist(generator);
            while (j > 0 && airports[j] == airports[j - 1] || (j == flights_per_aircraft[i] - 1 && airports[j] == airports[0])) {
                airports[j] = airport_dist(generator);
            }
        }
        airports[flights_per_aircraft[i]] = airports[0];

        // calculating total flight time
        std::vector<Flight> flights1 = GetAircraftFlights(
                airports,
                distances,
                aircraft_speed,
                hours_in_cycle,
                hour_size,
                min_passengers,
                max_passengers
        );
        flights.append_range(flights1);

        std::reverse(flights1.begin(), flights1.end());
        std::vector<Flight> flights2;
        flights2.reserve(flights_per_aircraft[i]);
        // generating symmetric flights
        std::uniform_int_distribution<int> start_index_dist(0, flights_per_aircraft[i] - 1);
        int start_index = start_index_dist(generator);
//        int start_index = 0;
        for (int j = 0; j < flights_per_aircraft[i]; ++j) {
            flights2.push_back(flights1[start_index]);
            start_index = (start_index + 1) % flights_per_aircraft[i];
        }

        // rewriting flight times
        auto total_flight_time = GetTotalFlightTime(airports, distances, aircraft_speed, hour_size);
        for (int j = 0; j < flights_per_aircraft[i]; ++j) {
            std::swap(flights2[j].departure_airport, flights2[j].arrival_airport);
            int from = flights2[j].departure_airport;
            int to = flights2[j].arrival_airport;
            int flight_time = static_cast<int>(std::ceil(
                    static_cast<double>(distances[from][to]) / aircraft_speed));
            int min_start_time = j == 0 ? hour_size : (flights2[j - 1].arrival_time + 2 * hour_size);
            int max_start_time = hours_in_cycle - total_flight_time + hour_size;
            std::uniform_int_distribution<int> departure_time_dist(min_start_time, max_start_time);
            flights2[j].departure_time = departure_time_dist(generator);
            flights2[j].arrival_time = flights2[j].departure_time + flight_time;
            total_flight_time -= flight_time + 2 * hour_size;
        }

        // adding new flights to flights vector
        flights.append_range(flights2);
    }
    return {flights, flights_per_aircraft};
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

std::vector<Aircraft> GenerateAircrafts(int aircrafts_number,
                                        const std::vector<int>& flights_per_aircraft,
                                        const std::vector<Flight>& flights,
                                        int min_passengers,
                                        int max_passengers,
                                        int min_cost,
                                        int max_cost) {
    std::vector<Aircraft> aircrafts;
    aircrafts.reserve(aircrafts_number);
    int aircraft_flights_start = 0;
    for (int i = 0; i < aircrafts_number; ++i) {
        int aircraft_min_passengers = 0;
        for (int j = 0; j < flights_per_aircraft[i]; ++j) {
            aircraft_min_passengers = std::max(
                    aircraft_min_passengers,
                    flights[aircraft_flights_start + j].min_passengers);
        }
        min_passengers = std::max(min_passengers, aircraft_min_passengers);
        std::uniform_int_distribution<int> passengers_dist(min_passengers, max_passengers);
        std::uniform_int_distribution<int> cost_dist(min_cost, max_cost);
        aircrafts.push_back({i, passengers_dist(generator), cost_dist(generator)});
        aircraft_flights_start += flights_per_aircraft[i];
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
        std::cerr << "parsing input" << std::endl;
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
    int hour_size = pObject->getValue<int>("hour size");
    int min_distance = pObject->getValue<int>("min distance");
    int max_distance = pObject->getValue<int>("max distance");
    int aircraft_speed = pObject->getValue<int>("aircraft speed");
    int aircrafts_number = pObject->getValue<int>("aircrafts number");
    int min_flights_per_aircraft = pObject->getValue<int>("min flights number per aircraft");
    int max_flights_per_aircraft = pObject->getValue<int>("max flights number per aircraft");
    int min_passengers = pObject->getValue<int>("min passengers number");
    int max_passengers = pObject->getValue<int>("max passengers number");
    int min_flight_cost = pObject->getValue<int>("min flight cost");
    int max_flight_cost = pObject->getValue<int>("max flight cost");
    int min_stay_cost = pObject->getValue<int>("min stay cost");
    int max_stay_cost = pObject->getValue<int>("max stay cost");

//    std::ifstream read_graph("../data/graph.txt");
//    std::vector<std::vector<int>> graph(airports_number, std::vector<int>(airports_number));
//    for (auto& edges_list : graph) {
//        for (auto& edges_number : edges_list) {
//            read_graph >> edges_number;
//        }
//    }

//    std::cerr << "before getting flights" << std::endl;
    auto [flights, flights_per_aircraft] = GenerateFlights(
            airports_number,
            aircrafts_number,
            hours_in_cycle,
            hour_size,
            min_distance,
            max_distance,
            aircraft_speed,
            flights_number,
            min_passengers,
            max_passengers - 400,
            min_flights_per_aircraft,
            max_flights_per_aircraft);
//    auto fleets = GenerateFleets(fleets_number, aircrafts_number, min_passengers + 400,
//                                 max_passengers, min_flight_cost, max_flight_cost);
//    std::cerr << "got flights" << std::endl;
    auto aircrafts = GenerateAircrafts(
            aircrafts_number,
            flights_per_aircraft,
            flights,
            min_passengers,
            max_passengers,
            min_flight_cost,
            max_flight_cost);
//    std::cerr << "got aircrafts" << std::endl;
    auto airports = GenerateAirports(airports_number, min_stay_cost, max_stay_cost);
//    std::cerr << "got airports" << std::endl;

    // shuffling flights, assigning id-s
    std::shuffle(flights.begin(), flights.end(), generator);
    for (int i = 0; i < flights.size(); ++i) {
        flights[i].id = i;
    }

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
