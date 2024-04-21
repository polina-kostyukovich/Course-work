#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include <Poco/JSON/JSON.h>
#include <Poco/JSON/Parser.h>

using namespace Poco::JSON;

#include "help_structs/entities.h"
#include "heuristic_model/local_search_solver.h"

struct BaseInputData {
    int flights_number;
    int aircrafts_number;
};


BaseInputData ReadBaseData() {
    std::ifstream read_input_data("../input_data/input.json");
    std::string input_json_string, string;
    while (std::getline(read_input_data, string)) {
        input_json_string += string + '\n';
    }

    Parser parser;
    Object::Ptr pObject = parser.parse(input_json_string).extract<Object::Ptr>();
    int flights_number = pObject->getValue<int>("flights number");
    int aircrafts_number = pObject->getValue<int>("aircrafts number");
    return {flights_number, aircrafts_number};
}

std::unique_ptr<InputData> ReadData() {
    std::ifstream read_input_data("../input_data/input.json");
    std::string input_json_string, string;
    while (std::getline(read_input_data, string)) {
        input_json_string += string + '\n';
    }

    Parser parser;
    Object::Ptr pObject = parser.parse(input_json_string).extract<Object::Ptr>();
    int airports_number = pObject->getValue<int>("airports number");
    int flights_number = pObject->getValue<int>("flights number");
    int aircrafts_number = pObject->getValue<int>("aircrafts number");
    int hours_in_cycle = pObject->getValue<int>("hours in cycle");

    std::ifstream read_data("../data/testing_data.json");
    std::string data_json_string;
    while (std::getline(read_data, string)) {
        data_json_string += string + '\n';
    }

    auto data = std::make_unique<InputData>();
    pObject = parser.parse(data_json_string).extract<Object::Ptr>();
    Array::Ptr flights_json = pObject->get("flights").extract<Array::Ptr>();
    data->flights = std::make_shared<std::vector<Flight>>();
    data->flights->reserve(flights_number);
    for (int i = 0; i < flights_number; ++i) {
        auto flight = flights_json->getObject(i);
        data->flights->emplace_back(flight->getValue<int>("id"),
                                   flight->getValue<int>("departure airport"),
                                   flight->getValue<int>("arrival airport"),
                                   flight->getValue<int>("departure time"),
                                   flight->getValue<int>("arrival time"),
                                   flight->getValue<int>("distance"),
                                   flight->getValue<int>("min passengers"));
    }

    Array::Ptr aircrafts_json = pObject->get("aircrafts").extract<Array::Ptr>();
    data->aircrafts = std::make_shared<std::vector<Aircraft>>();
    data->aircrafts->reserve(aircrafts_number);
    for (int i = 0; i < aircrafts_number; ++i) {
        auto aircraft = aircrafts_json->getObject(i);
        data->aircrafts->emplace_back(aircraft->getValue<int>("id"),
                                     aircraft->getValue<int>("seats"),
                                     aircraft->getValue<int>("flight cost"));
    }

    Array::Ptr airports_json = pObject->get("airports").extract<Array::Ptr>();
    data->airports = std::make_shared<std::vector<Airport>>();
    data->airports->reserve(airports_number);
    for (int i = 0; i < airports_number; ++i) {
        auto airport = airports_json->getObject(i);
        data->airports->emplace_back(airport->getValue<int>("id"),
                                    airport->getValue<int>("stay cost"));
    }

    data->hours_in_cycle = hours_in_cycle;
    return data;
}


void PrintSolution(const std::vector<int>& solution,
                   const SolutionCorrectnessInfo& status,
                   int aircrafts_number,
                   std::ostream& out) {
    if (status.IsCorrect()) {
        out << "Feasible solution\n";
    } else {
        out << "Infeasible solution: ";
        if (status.flights_intersect) {
            out << "flights intersect\n";
        } else if (status.airports_mismatch) {
            out << "airports mismatch\n";
        } else if (status.seats_lack) {
            out << "seats lack\n";
        }
    }

    out << "Solution:\n";
    out << "Flight" << '\t' << "Aircraft" << '\n';
    for (int i = 0; i < solution.size(); ++i) {
        if (solution[i] < aircrafts_number) {
            out << i << '\t' << solution[i] << '\n';
        } else {
            out << i << '\t' << "NO" << '\n';
        }
    }
}

int main() {
    const int kIterationsNumber = 1'000'000;
    const int kSeed = 1234;
    auto input_data = ReadData();
    std::cout << "read data" << std::endl;

    auto local_search_solver = LocalSearchSolver(input_data, kIterationsNumber, kSeed, true);
    auto solution = local_search_solver.Solve();
    auto solution_status = local_search_solver.GetCorrectnessInfo();
    PrintSolution(solution, solution_status, input_data->aircrafts->size(), std::cout);
    return 0;
}
