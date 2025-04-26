#include <fstream>
#include <iostream>

#include <boost/process.hpp>

int main() {
    const int kTestSuitsNumber = 8;
    const int kExperimentsNumber = 15;

    for (int i = 0; i < kTestSuitsNumber; ++i) {
//        std::string input_file = "../test_data/0_5/input.json";
        std::string input_file = "../test_data/bad_test_data/" + std::to_string(i) + "/input.json";
        for (int iteration = 0; iteration < kExperimentsNumber; ++iteration) {
            std::cout << "iteration " << iteration << std::endl;
            std::string file_id = std::to_string(i) + '_' + std::to_string(iteration);
//            std::string data_file = " ../test_data/0_5/test" + file_id + ".json";
            std::string data_file = " ../test_data/bad_test_data/" + std::to_string(i) + "/test" + file_id + ".json";
//            std::string output_file = "../cp_test_data/0_5/test" + file_id + ".dzn";
            std::string output_file = "../constraint_programming/cp_bad_test_data/" + std::to_string(i) + "/test" + file_id + ".dzn";
            auto data_converting =
                    boost::process::child("convert_data_for_cp",
                                          input_file + " " + data_file + " " + output_file);
            data_converting.wait();
        }
    }
    return 0;
}
