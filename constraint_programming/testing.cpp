#include <iostream>
#include <fstream>

#include <boost/process.hpp>

int main() {
    const int kTestSuit = 0;
    const int kTestsNumber = 15;

    std::string results_file = "../constraint_programming/tests_results/0_5.txt";
    std::ofstream write(results_file);
    for (int test = 1; test < kTestsNumber; ++test) {
        std::string model_file = "../constraint_programming/first_model.mzn";
        std::string file_id = std::to_string(kTestSuit) + "_" + std::to_string(test);
        std::string data_file = "../cp_test_data/0_5/test" + file_id + ".dzn";
//        std::string data_file = "../cp_test_data/" + std::to_string(kTestSuit) + "/test" + file_id + ".dzn";
//        std::cout << "starting" << std::endl;
        boost::process::ipstream is; //reading pipe-stream
        auto solution = boost::process::child("minizinc -s " + model_file + " " + data_file,
                                              boost::process::std_out > is);
//        auto solution = boost::process::child("ls -l -a");

        std::string line;
        write << "TEST " << test << "\n";
        while (std::getline(is, line)) {
            write << line << '\n';
        }
//        std::cout << "end writing" << std::endl;

        solution.wait();
        write << "\n\n\n";
        write.flush();
    }
    return 0;
}
