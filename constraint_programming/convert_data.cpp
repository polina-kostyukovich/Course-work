#include <algorithm>
#include <fstream>
#include <iostream>

#include "../help_structs/data_reader.h"
#include "../help_structs/entities.h"


// argv[1] --- input_filename data_filename converted_output_filename
int main(int argc, char** argv) {
    std::string input_filename;
    std::string data_filename;
    std::string converted_data_filename;
    if (argc == 2) {
        auto args = GetArgs(argv[1]);
        input_filename = args[0];
        data_filename = args[1];
        converted_data_filename = args[2];
    } else {
        throw;
    }

    auto reader = DataReader(input_filename, data_filename);
    auto input_data = reader.ReadData();

    std::ofstream write(converted_data_filename);
    // general data
    write << "start_time = 0;\n";
    write << "finish_time = " << input_data->hours_in_cycle << ";\n";
    write << "hour_size = " << input_data->hour_size << ";\n\n";

    write << "flights_number = " << input_data->flights->size() << ";\n";
    write << "aircrafts_number = " << input_data->aircrafts->size() << ";\n";
    write << "airports_number = " << input_data->airports->size() << ";\n\n";

    // sort flights
    std::sort(input_data->flights->begin(), input_data->flights->end(), [](const auto& flight1, const auto& flight2){
        return flight1.departure_time < flight2.departure_time;
    });

    // flights data
    std::vector<std::string> flights_strs(6);
    for (auto& str : flights_strs) {
        str.reserve(10 * input_data->flights->size());
    }
    for (const auto& flight : *input_data->flights) {
        flights_strs[0] += std::to_string(flight.departure_airport + 1) + ", ";
        flights_strs[1] += std::to_string(flight.arrival_airport + 1) + ", ";
        flights_strs[2] += std::to_string(flight.departure_time - input_data->hour_size) + ", ";
        flights_strs[3] += std::to_string(flight.arrival_time + input_data->hour_size) + ", ";
        flights_strs[4] += std::to_string(flight.distance) + ", ";
        flights_strs[5] += std::to_string(flight.min_passengers) + ", ";
    }
    for (auto& str : flights_strs) {
        str.pop_back();
        str.pop_back();
    }
    write << "flights_departure_points = [" << flights_strs[0] << "];\n";
    write << "flights_arrival_points = [" << flights_strs[1] << "];\n";
    write << "flights_departure_times = [" << flights_strs[2] << "];\n";
    write << "flights_arrival_times = [" << flights_strs[3] << "];\n";
    write << "flights_distances = [" << flights_strs[4] << "];\n";
    write << "flights_min_passangers = [" << flights_strs[5] << "];\n\n";

    // aircrafts data
    std::vector<std::string> aircrafts_strs(2);
    for (auto& str : aircrafts_strs) {
        str.reserve(10 * input_data->aircrafts->size());
    }
    for (const auto& aircraft : *input_data->aircrafts) {
        aircrafts_strs[0] += std::to_string(aircraft.seats) + ", ";
        aircrafts_strs[1] += std::to_string(aircraft.flight_cost) + ", ";
    }
    for (auto& str : aircrafts_strs) {
        str.pop_back();
        str.pop_back();
    }
    write << "aircrafts_seats = [" << aircrafts_strs[0] << "];\n";
    write << "aircrafts_costs = [" << aircrafts_strs[1] << "];\n\n";

    // airports_data
    std::string airports_str;
    airports_str.reserve(10 * input_data->airports->size());
    for (const auto& airport : *input_data->airports) {
        airports_str += std::to_string(airport.stay_cost) + ", ";
    }
    airports_str.pop_back();
    airports_str.pop_back();
    write << "airports_costs = [" << airports_str << "];\n";
    return 0;
}