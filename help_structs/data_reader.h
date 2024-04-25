#pragma once

#include <string>

#include "entities.h"

class DataReader {
public:
    DataReader(const std::string& input_filname, const std::string& data_filename);

    BaseInputData ReadBaseData() const;
    std::unique_ptr<InputData> ReadData() const;

private:
    std::string input_filename_;
    std::string data_filename_;
};
