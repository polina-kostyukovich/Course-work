#include <fstream>
#include <iostream>
#include <random>

#include <boost/process.hpp>
#include <Poco/JSON/JSON.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

#include "help_structs/entities.h"

using namespace Poco::JSON;

int main() {
    std::vector<int> airports_numbers = {3, 5, 7, 10, 12, 15, 20, 25};
//    std::vector<int> airports_numbers = {15};
    std::vector<int> flights_numbers = {10, 16, 22, 30, 36, 46, 60, 76};
//    std::vector<int> flights_numbers = {46};
    std::vector<int> aircrafts_numbers = {3, 5, 7, 10, 12, 15, 20, 25};
//    std::vector<int> aircrafts_numbers = {15};
    const int kExperimentsNumber = 15;

    std::mt19937 generator(42);
    std::uniform_int_distribution<int> seed_dist(1, 1'000'000);

    Poco::JSON::Object::Ptr data = new Poco::JSON::Object;
    data->set("hours in cycle", 10'080);  // min
    data->set("hour size", 60);  // min
    data->set("min distance", 200);
    data->set("max distance", 2000);
    data->set("aircraft speed", 14);  // km/min
    data->set("min passengers number", 100);
    data->set("max passengers number", 1000);
    data->set("min flight cost", 1);
    data->set("max flight cost", 5);
    data->set("min stay cost", 1);
    data->set("max stay cost", 10);
    data->set("min flights number per aircraft", 2);
    data->set("max flights number per aircraft", 4);

    for (int i = 0; i < airports_numbers.size(); ++i) {
        int airports_number = airports_numbers[i];
        std::cout << "airports number: " << airports_number << '\n';
        int flights_number = flights_numbers[i];
        int aircrafts_number = aircrafts_numbers[i];
        int fleets_number = std::max(2, aircrafts_number / 5);
        data->set("flights number", flights_number);
        data->set("aircrafts number", aircrafts_number);
        data->set("airports number", airports_number);
        data->set("fleets number", fleets_number);

//        std::string input_file = std::string("../test_data/") + "0" + "/input.json";
        std::string input_file = "../test_data/" + std::to_string(i) + "/input.json";
        std::ofstream oss(input_file);
        Poco::JSON::Stringifier::stringify(data, oss, 4, -1, Poco::JSON_PRESERVE_KEY_ORDER);
        oss.flush();
        oss.close();

        for (int iteration = 0; iteration < kExperimentsNumber; ++iteration) {
            std::cout << "iteration " << iteration << std::endl;
            std::string file_id = std::to_string(i) + '_' + std::to_string(iteration);
//            std::cout << "zero" << std::endl;
//            auto vertices_degrees_generation =
//                boost::process::child("vertices_degrees_generation",
//                                      std::to_string(seed_dist(generator)) + " " + input_file);
//            vertices_degrees_generation.wait();
//            std::cout << "first" << std::endl;
//            auto graph_generation =
//                boost::process::child("graph_generation",
//                                      std::to_string(seed_dist(generator)));
//            graph_generation.wait();
//            std::cout << "second" << std::endl;
            auto data_generation =
                boost::process::child("data_generation",
                                      std::to_string(seed_dist(generator)) + " " + input_file
//                                        + " ../test_data/" + "0" + "/test" + file_id + ".json");
                                        + " ../test_data/" + std::to_string(i) + "/test" + file_id + ".json");
            data_generation.wait();
        }
    }
    return 0;
}
