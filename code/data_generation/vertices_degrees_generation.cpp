#include <fstream>
#include <iostream>
#include <random>
#include <vector>

#include <Poco/JSON/JSON.h>
#include <Poco/JSON/Parser.h>

using namespace Poco::JSON;

std::vector<int> GetRandomVerticesDegrees(int vertices_number, int degrees_sum) {
    std::mt19937 generator(2873);
    std::vector<int> degrees(vertices_number, 1);
    degrees_sum -= vertices_number;
    std::uniform_int_distribution<int> index_dist(0, vertices_number - 1);
    for (int i = 0; i < degrees_sum; ++i) {
        int random_index = index_dist(generator);
        degrees[random_index] += 1;
    }
    return degrees;
}

int main() {
    std::ifstream read("../input_data/input.json");
    std::string json_string, string;
    while (std::getline(read, string)) {
        json_string += string + '\n';
    }

    Parser parser;
    Object::Ptr pObject = parser.parse(json_string).extract<Object::Ptr>();
    int airports_number = pObject->getValue<int>("airports number");
    int degrees_sum = pObject->getValue<int>("flights number");

    auto degrees = GetRandomVerticesDegrees(airports_number, degrees_sum);

    std::ofstream write("../data/vertices_degrees.txt");
    write << airports_number << '\n';
    for (auto degree : degrees) {
        write << degree << ' ';
    }
    write << '\n';
    return 0;
}
