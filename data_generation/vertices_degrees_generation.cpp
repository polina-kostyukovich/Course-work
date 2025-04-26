#include <fstream>
#include <iostream>
#include <random>
#include <vector>

#include <Poco/JSON/JSON.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

#include "../help_structs/entities.h"

using namespace Poco::JSON;

std::vector<int> GetRandomVerticesDegrees(int vertices_number, int degrees_sum, int seed) {
    std::mt19937 generator(seed);
    std::vector<int> degrees(vertices_number, 0);
    if (vertices_number <= degrees_sum) {
        degrees = std::vector<int>(vertices_number, 1);
        degrees_sum -= vertices_number;
    } else {
        for (int i = 0; i < degrees_sum; ++i) {
            degrees[i] = 1;
        }
        return degrees;
    }
    std::uniform_int_distribution<int> index_dist(0, vertices_number - 1);
    for (int i = 0; i < degrees_sum; ++i) {
        int random_index = index_dist(generator);
        degrees[random_index] += 1;
        auto sum = std::accumulate(degrees.begin(), degrees.end(), 0);
        if (2 * degrees[random_index] > sum) {
            degrees[random_index] -= 1;
            i -= 1;
        }
    }
    return degrees;
}

int main(int argc, char** argv) {
    int seed = 139;
    std::string input_filename = "../input_data/input.json";
    if (argc == 2) {
        auto args = GetArgs(argv[1]);
        seed = std::stoi(args[0]);
        input_filename = args[1];
    }

    std::ifstream read(input_filename);
    std::string json_string, string;
    while (std::getline(read, string)) {
        json_string += string + '\n';
    }

    Parser parser;
    Object::Ptr pObject = parser.parse(json_string).extract<Object::Ptr>();
    int airports_number = pObject->getValue<int>("airports number");
    int degrees_sum = pObject->getValue<int>("flights number");

    auto degrees = GetRandomVerticesDegrees(airports_number, degrees_sum, seed);

    std::ofstream write("vertices_degrees.txt");
    write << airports_number << '\n';
    for (auto degree : degrees) {
        write << degree << ' ';
    }
    write << std::endl;
    return 0;
}
