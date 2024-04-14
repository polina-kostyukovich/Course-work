#include <fstream>
#include <iostream>
#include <random>

#include <boost/process.hpp>
#include <Poco/JSON/JSON.h>
#include <Poco/JSON/Parser.h>

using namespace Poco::JSON;

int main() {
    std::mt19937 generator(36587);
    std::uniform_int_distribution<int> seed_dist(1, 1'000'000);

    Object::Ptr data = new Poco::JSON::Object;
    data->set("hours in cycle", 96);
    data->set("min distance", 200);
    data->set("max distance", 2000);
    data->set("aircraft speed", 300);
    data->set("min passengers number", 100);
    data->set("max passengers number", 1000);
    data->set("min flight cost", 1);
    data->set("max flight cost", 5);
    data->set("min stay cost", 10);
    data->set("max stay cost", 100);

    std::ofstream result("../results.txt");

    for (int i = 0; i < 2; ++i) {
        for (int aircrafts_number = 2; aircrafts_number < 3; ++aircrafts_number) {
            int flights_number = aircrafts_number * 4;
            if (flights_number % 2 == 1) {
                flights_number += 1;
            }
            int airports_number = std::max(3, (flights_number + 7) / 8);
            int fleets_number = std::max(2, aircrafts_number / 5);
            data->set("flights number", flights_number);
            data->set("aircrafts number", aircrafts_number);
            data->set("airports number", airports_number);
            data->set("fleets number", fleets_number);

            std::ofstream oss("../input_data/input.json");
            Poco::JSON::Stringifier::stringify(data, oss, 4, -1, Poco::JSON_PRESERVE_KEY_ORDER);
            oss.flush();

            // std::cout << "zero" << std::endl;
            auto vertices_degrees_generation = boost::process::child("vertices_degrees_generation",
                                                                     std::to_string(seed_dist(
                                                                         generator)));
            vertices_degrees_generation.wait();
            // std::cout << "first" << std::endl;
            auto graph_generation =
                boost::process::child("graph_generation", std::to_string(seed_dist(generator)));
            graph_generation.wait();
            // std::cout << "second" << std::endl;
            auto data_generation =
                boost::process::child("data_generation", std::to_string(seed_dist(generator)));
            data_generation.wait();
            // std::cout << "third" << std::endl;
            boost::process::ipstream is; //reading pipe-stream
            boost::process::ipstream error; //reading pipe-stream
            auto solution = boost::process::child("solution",
                                                  boost::process::std_out > is);

            std::string line;
            while (solution.running() && std::getline(is, line)) {
                result << line << '\n';
            }

            solution.wait();
            result.flush();
            // std::cout << "fourth" << std::endl;
        }
    }
    return 0;
}
