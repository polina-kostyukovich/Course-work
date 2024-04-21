#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include <Highs.h>
#include <Poco/JSON/JSON.h>
#include <Poco/JSON/Parser.h>

using namespace Poco::JSON;

#include "help_structs/entities.h"
#include "help_structs/indices_converter.h"
#include "help_structs/row_wise_matrix.h"

struct BaseInputData {
    int flights_number;
    int aircrafts_number;
};

struct ProcessedData {
    std::vector<int> time_points;
    std::vector<size_t> departure_times_indices;
    std::vector<size_t> arrival_times_indices;
    std::vector<std::vector<int>> is_flight_flying;
    std::vector<std::vector<int>> departure_airports_indicator;
    std::vector<std::vector<int>> arrival_airports_indicator;

    void Print() const {
        std::cout << "time_points:" << std::endl;
        for (auto item : time_points) {
            std::cout << item << ' ';
        }
        std::cout << "\ndeparture_times_indices\n";
        for (auto item : departure_times_indices) {
            std::cout << item << ' ';
        }
        std::cout << "\narrival_times_indices\n";
        for (auto item : arrival_times_indices) {
            std::cout << item << ' ';
        }
        std::cout << "\nis_flight_flying\n";
        for (auto& array : is_flight_flying) {
            for (auto item : array) {
                std::cout << item << " ";
            }
            std::cout << '\n';
        }
        std::cout << "\ndeparture_airports_indicator\n";
        for (auto& array : departure_airports_indicator) {
            for (auto item : array) {
                std::cout << item << " ";
            }
            std::cout << '\n';
        }
        std::cout << "\narrival_airports_indicator\n";
        for (auto& array : arrival_airports_indicator) {
            for (auto item : array) {
                std::cout << item << " ";
            }
            std::cout << '\n';
        }
    }
};

void MergeIntoArray(const std::vector<int>& array1, const std::vector<int>& array2,
                    std::vector<int>& array_to_fill) {
    size_t index1 = 0;
    size_t index2 = 0;
    while (index1 < array1.size() && index2 < array2.size()) {
        if (array1[index1] <= array2[index2]) {
            array_to_fill.push_back(array1[index1++]);
        } else {
            array_to_fill.push_back(array2[index2++]);
        }
    }

    while (index1 < array1.size()) {
        array_to_fill.push_back(array1[index1++]);
    }
    while (index2 < array2.size()) {
        array_to_fill.push_back(array2[index2++]);
    }
}

std::vector<int> GetTimePointsArray(const std::unique_ptr<InputData>& input_data) {
    std::vector<int> departure_times;
    std::vector<int> arrival_times;
    departure_times.reserve(input_data->flights->size());
    arrival_times.reserve(input_data->flights->size());
    for (auto& flight : (*input_data->flights)) {
        departure_times.push_back(flight.departure_time - 1);
        arrival_times.push_back(flight.arrival_time + 1);
    }

    std::sort(departure_times.begin(), departure_times.end());
    std::sort(arrival_times.begin(), arrival_times.end());
    auto departure_end = std::unique(departure_times.begin(), departure_times.end());
    departure_times.resize(departure_end - departure_times.begin());
    auto arrival_end = std::unique(arrival_times.begin(), arrival_times.end());
    departure_times.resize(arrival_end - arrival_times.begin());

    std::vector<int> times = {0};
    times.reserve(input_data->flights->size() + 2);
    MergeIntoArray(departure_times, arrival_times, times);
    times.push_back(input_data->hours_in_cycle);
    return times;
}

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
    data->flights = std::make_shared<std::vector<Flight>>();
    data->aircrafts = std::make_shared<std::vector<Aircraft>>();
    data->airports = std::make_shared<std::vector<Airport>>();

    pObject = parser.parse(data_json_string).extract<Object::Ptr>();
    Array::Ptr flights_json = pObject->get("flights").extract<Array::Ptr>();
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
    data->aircrafts->reserve(aircrafts_number);
    for (int i = 0; i < aircrafts_number; ++i) {
        auto aircraft = aircrafts_json->getObject(i);
        data->aircrafts->emplace_back(aircraft->getValue<int>("id"),
                                     aircraft->getValue<int>("seats"),
                                     aircraft->getValue<int>("flight cost"));
    }

    Array::Ptr airports_json = pObject->get("airports").extract<Array::Ptr>();
    data->airports->reserve(airports_number);
    for (int i = 0; i < airports_number; ++i) {
        auto airport = airports_json->getObject(i);
        data->airports->emplace_back(airport->getValue<int>("id"),
                                    airport->getValue<int>("stay cost"));
    }

    data->hours_in_cycle = hours_in_cycle;
    return data;
}

std::unique_ptr<ProcessedData> GetProcessedData(const std::unique_ptr<InputData>& input_data) {
    auto processed_data = std::make_unique<ProcessedData>();
    processed_data->time_points = GetTimePointsArray(input_data);
    processed_data->is_flight_flying = std::vector<std::vector<int>>(
        input_data->flights->size(),
        std::vector<int>(processed_data->time_points.size() - 1, 0)
    );
    processed_data->departure_times_indices.resize(input_data->flights->size());
    processed_data->arrival_times_indices.resize(input_data->flights->size());

    for (size_t i = 0; i < input_data->flights->size(); ++i) {
        auto flight = (*input_data->flights)[i];
        auto departure_time_index = std::upper_bound(processed_data->time_points.begin(),
                                                     processed_data->time_points.end(),
                                                     flight.departure_time - 1)
            - processed_data->time_points.begin() - 1;

        auto arrival_time_index = std::lower_bound(processed_data->time_points.begin(),
                                                   processed_data->time_points.end(),
                                                   flight.arrival_time + 1)
            - processed_data->time_points.begin();

        for (auto j = departure_time_index; j < arrival_time_index; ++j) {
            processed_data->is_flight_flying[i][j] = 1;
        }

        processed_data->departure_times_indices[i] = departure_time_index;
        processed_data->arrival_times_indices[i] = arrival_time_index;
    }

    processed_data->departure_airports_indicator = std::vector<std::vector<int>>(
        input_data->flights->size(),
        std::vector<int>(input_data->airports->size(), 0)
    );
    processed_data->arrival_airports_indicator = std::vector<std::vector<int>>(
        input_data->flights->size(),
        std::vector<int>(input_data->airports->size(), 0)
    );

    for (size_t i = 0; i < input_data->flights->size(); ++i) {
        auto flight = (*input_data->flights)[i];
        processed_data->departure_airports_indicator[i][flight.departure_airport] = 1;
        processed_data->arrival_airports_indicator[i][flight.arrival_airport] = 1;
    }

    return processed_data;
}

size_t GetNeededCapacity(size_t i_number, size_t j_number, size_t l_number, size_t k_number,
                         const std::unique_ptr<ProcessedData>& processed_data) {
    size_t capacity = 0;
    // 1 restriction
    capacity += i_number * j_number;
    // 2 restriction
    capacity += i_number * j_number;
    // 4 restriction
    capacity += 4 * i_number * j_number * l_number;
    // 5 restriction
    capacity += 4 * i_number * j_number * l_number;
    // 7 restriction
    size_t F_sum = 0;
    for (size_t i = 0; i < i_number; ++i) {
        for (size_t k = 0; k < k_number; ++k) {
            F_sum += processed_data->is_flight_flying[i][k];
        }
    }
    capacity += 2 * (2 * l_number * j_number * k_number + 2 * j_number * l_number * F_sum);
    // 9 restriction
    capacity += 2 * j_number * l_number;
    // 10 restriction
    capacity += j_number * F_sum;
    // 11 restriction
    capacity += l_number * j_number * k_number + j_number * F_sum;

    return capacity;
}

std::unique_ptr<Restrictions> GetModelRestrictions(
    const std::unique_ptr<InputData>& input_data,
    const std::unique_ptr<ProcessedData>& processed_data,
    const double M) {
    size_t i_number = input_data->flights->size();
    size_t j_number = input_data->aircrafts->size();
    size_t l_number = input_data->airports->size();
    size_t k_number = processed_data->time_points.size() - 1;
    std::cout << "i: " << i_number << std::endl;
    std::cout << "j: " << j_number << std::endl;
    std::cout << "l: " << l_number << std::endl;
    std::cout << "k: " << k_number << std::endl;
    IndicesConverter indices_converter{i_number, j_number, l_number, k_number};
    auto restrictions = std::make_unique<Restrictions>();
    restrictions->SetColumnsNumber(indices_converter.IndicesNumber());
    auto needed_capacity = GetNeededCapacity(i_number, j_number, l_number, k_number, processed_data);
    restrictions->Reserve(needed_capacity);

    // 1 restriction
    for (size_t i = 0; i < i_number; ++i) {
        Row row;
        row.non_zero_values = std::vector<double>(j_number, 1.0);
        row.columns_indices.reserve(j_number);
        for (size_t j = 0; j < j_number; ++j) {
            row.columns_indices.push_back(indices_converter.ConvertXIndex(i, j));
        }
        row.low_bound = 1.0;
        row.up_bound = 1.0;
        restrictions->AddRow(row);
    }
    // std::cout << "Got 1 restriction" << std::endl;

    // 2 restriction
    for (size_t i = 0; i < i_number; ++i) {
        for (size_t j = 0; j < j_number; ++j) {
            if ((*input_data->aircrafts)[j].seats - (*input_data->flights)[i].min_passengers == 0) {
                continue;
            }

            Row row;
            row.non_zero_values = {static_cast<double>((*input_data->aircrafts)[j].seats
                - (*input_data->flights)[i].min_passengers)};
            row.columns_indices = {indices_converter.ConvertXIndex(i, j)};
            row.low_bound = 0;
            restrictions->AddRow(row);
        }
    }
    // std::cout << "Got 2 restriction" << std::endl;

    // 4 restriction
    for (size_t i = 0; i < i_number; ++i) {
        for (size_t j = 0; j < j_number; ++j) {
            for (size_t l = 0; l < l_number; ++l) {
                size_t k = processed_data->departure_times_indices[i] - 1;
                Row row;
                row.non_zero_values = {M, 1};
                row.columns_indices = {indices_converter.ConvertXIndex(i, j),
                                       indices_converter.ConvertAIndex(l, j, k)};
                row.up_bound = M + processed_data->departure_airports_indicator[i][l];

                restrictions->AddRow(row);
                row.non_zero_values.back() = -1;
                row.up_bound = M - processed_data->departure_airports_indicator[i][l];
                restrictions->AddRow(row);
            }
        }
    }
    // std::cout << "Got 4 restriction" << std::endl;

    // 5 restriction
    for (size_t i = 0; i < i_number; ++i) {
        for (size_t j = 0; j < j_number; ++j) {
            for (size_t l = 0; l < l_number; ++l) {
                size_t k = processed_data->arrival_times_indices[i];
                Row row;
                row.non_zero_values = {M, 1};
                row.columns_indices = {indices_converter.ConvertXIndex(i, j),
                                       indices_converter.ConvertAIndex(l, j, k)};
                row.up_bound = M + processed_data->arrival_airports_indicator[i][l];

                restrictions->AddRow(row);
                row.non_zero_values.back() = -1;
                row.up_bound = M - processed_data->arrival_airports_indicator[i][l];
                restrictions->AddRow(row);
            }
        }
    }
    // std::cout << "Got 5 restriction" << std::endl;

    // 7 restriction
    for (size_t j = 0; j < j_number; ++j) {
        for (size_t k = 0; k < k_number - 1; ++k) {
            Row row;
            for (size_t i = 0; i < i_number; ++i) {
                if (processed_data->is_flight_flying[i][k]
                    + processed_data->is_flight_flying[i][k + 1] == 0) {
                    continue;
                }

                row.non_zero_values.push_back(M * (processed_data->is_flight_flying[i][k]
                    + processed_data->is_flight_flying[i][k + 1]));
                row.columns_indices.push_back(indices_converter.ConvertXIndex(i, j));
            }
            row.non_zero_values.push_back(1);
            row.non_zero_values.push_back(-1);
            row.columns_indices.resize(row.columns_indices.size() + 2);
            row.low_bound = 0;

            for (size_t l = 0; l < l_number; ++l) {
                row.non_zero_values[row.non_zero_values.size() - 2] = 1;
                row.non_zero_values[row.non_zero_values.size() - 1] = -1;
                row.columns_indices[row.columns_indices.size() - 2] = indices_converter.ConvertAIndex(l, j, k);
                row.columns_indices[row.columns_indices.size() - 1] = indices_converter.ConvertAIndex(l, j, k + 1);
                restrictions->AddRow(row);

                row.non_zero_values[row.non_zero_values.size() - 2] = -1;
                row.non_zero_values[row.non_zero_values.size() - 1] = 1;
                restrictions->AddRow(row);
            }
        }
    }
    // std::cout << "Got 7 restriction" << std::endl;

    // 9 restriction
    for (size_t j = 0; j < j_number; ++j) {
        for (size_t l = 0; l < l_number; ++l) {
            Row row;
            row.non_zero_values = {1, -1};
            row.columns_indices = {indices_converter.ConvertAIndex(l, j, 0),
                                   indices_converter.ConvertAIndex(l, j, k_number - 1)};
            row.up_bound = 0;
            row.low_bound = 0;
            restrictions->AddRow(row);
        }
    }
    // std::cout << "Got 9 restriction" << std::endl;

    // 10 restriction
    for (size_t j = 0; j < j_number; ++j) {
        for (size_t k = 0; k < k_number; ++k) {
            Row row;
            for (size_t i = 0; i < i_number; ++i) {
                if (processed_data->is_flight_flying[i][k] == 0) {
                    continue;
                }
                row.non_zero_values.push_back(processed_data->is_flight_flying[i][k]);
                row.columns_indices.push_back(indices_converter.ConvertXIndex(i, j));
            }
            if (!row.non_zero_values.empty()) {
                row.up_bound = 1;
                restrictions->AddRow(row);
            }
        }
    }
    // std::cout << "Got 10 restriction" << std::endl;

    // 11 restriction
    for (size_t j = 0; j < j_number; ++j) {
        for (size_t k = 0; k < k_number; ++k) {
            Row row;
            row.non_zero_values.reserve(l_number);
            row.columns_indices.reserve(l_number);
            for (size_t i = 0; i < i_number; ++i) {
                if (processed_data->is_flight_flying[i][k] == 0) {
                    continue;
                }
                row.non_zero_values.push_back(processed_data->is_flight_flying[i][k]);
                row.columns_indices.push_back(indices_converter.ConvertXIndex(i, j));
            }
            for (size_t l = 0; l < l_number; ++l) {
                row.non_zero_values.push_back(1);
                row.columns_indices.push_back(indices_converter.ConvertAIndex(l, j, k));
            }
            row.up_bound = 1;
            row.low_bound = 1;
            restrictions->AddRow(row);
        }
    }
    // std::cout << "Got 11 restriction" << std::endl;

    return restrictions;
}

std::vector<double> GetTargetCoefficients(const std::unique_ptr<InputData>& input_data,
                                          const std::unique_ptr<ProcessedData>& processed_data) {
    size_t i_number = input_data->flights->size();
    size_t j_number = input_data->aircrafts->size();
    size_t l_number = input_data->airports->size();
    size_t k_number = processed_data->time_points.size() - 1;
    IndicesConverter indices_converter{i_number, j_number, l_number, k_number};
    std::vector<double> target_coefficients(indices_converter.IndicesNumber(), 0);

    // 1 sum
    for (size_t i = 0; i < i_number; ++i) {
        for (size_t j = 0; j < j_number; ++j) {
            auto index = indices_converter.ConvertXIndex(i, j);
            target_coefficients[index] = static_cast<double>((*input_data->aircrafts)[j].flight_cost) *
                (*input_data->flights)[i].distance;
        }
    }

    // 2 sum
    for (size_t j = 0; j < j_number; ++j) {
        for (size_t l = 0; l < l_number; ++l) {
            auto index = indices_converter.ConvertAIndex(l, j, 0);
            target_coefficients[index] = (*input_data->airports)[l].stay_cost
                * (processed_data->time_points[1] - processed_data->time_points[0] + 1);
            for (size_t k = 1; k < k_number - 1; ++k) {
                index = indices_converter.ConvertAIndex(l, j, k);
                target_coefficients[index] = (*input_data->airports)[l].stay_cost
                    * (processed_data->time_points[k + 1] - processed_data->time_points[k] + 2);
            }
            index = indices_converter.ConvertAIndex(l, j, k_number - 1);
            target_coefficients[index] = (*input_data->airports)[l].stay_cost
                * (processed_data->time_points[k_number] - processed_data->time_points[k_number - 1] + 1);
        }
    }

    return target_coefficients;
}

std::unique_ptr<HighsModel> GetHighsModel(const std::unique_ptr<Restrictions>& restrictions,
                                          const std::vector<double>& target_coefficients) {
    auto model = std::make_unique<HighsModel>();
    assert(restrictions->GetMatrix().GetValues().size() == restrictions->GetMatrix().GetColumnIndices().size());
    model->lp_.num_col_ = restrictions->GetMatrix().GetColumnsNumber();  // variables number
    model->lp_.num_row_ = restrictions->GetMatrix().GetRowsNumber();  // restrictions number
    model->lp_.sense_ = ObjSense::kMinimize;
    model->lp_.offset_ = 0;  // summand in target function
    model->lp_.col_cost_ = target_coefficients;  // target function coefficients
    model->lp_.col_lower_ = std::vector<double>(model->lp_.num_col_, 0);  // variables bounds
    model->lp_.col_upper_ = std::vector<double>(model->lp_.num_col_, 1);
    model->lp_.row_lower_ = restrictions->GetLowBounds();  // restrictions bounds
    model->lp_.row_upper_ = restrictions->GetUpBounds();

    model->lp_.a_matrix_.format_ = MatrixFormat::kRowwise;
    model->lp_.a_matrix_.value_ = restrictions->GetMatrix().GetValues();
    model->lp_.a_matrix_.index_ = restrictions->GetMatrix().GetColumnIndices();
    model->lp_.a_matrix_.start_ = restrictions->GetMatrix().GetRowsStarts();

    model->lp_.integrality_.resize(model->lp_.num_col_);
    for (auto& item : model->lp_.integrality_) {
        item = HighsVarType::kInteger;
    }
    return model;
}

int main() {
    const double kM = 1e10;
    auto base_input_data = ReadBaseData();
    auto input_data = ReadData();
    // std::cout << "Read data" << std::endl;
    auto processed_data = GetProcessedData(input_data);
    // std::cout << "Processed data" << std::endl;
    // processed_data->Print();
    auto model_restrictions = GetModelRestrictions(input_data, processed_data, kM);
    std::cout << "rows: " << model_restrictions->GetMatrix().GetRowsNumber() << '\n';
    std::cout << "cols: " << model_restrictions->GetMatrix().GetColumnsNumber() << '\n';
    std::cout << "coefs: " << model_restrictions->GetMatrix().GetValues().size() << std::endl;
    // std::cout << "Got restrictions" << std::endl;
    auto target_coefficients = GetTargetCoefficients(input_data, processed_data);
    // std::cout << "Got target coefficients" << std::endl;

    auto model = GetHighsModel(model_restrictions, target_coefficients);
    // std::cout << "Got model" << std::endl;
    input_data.reset();
    processed_data.reset();
    model_restrictions.reset();
    target_coefficients.clear();
    target_coefficients.shrink_to_fit();

    // std::cout << "start solving" << std::endl;
    // Start preparations for solving
    Highs highs;
    HighsStatus return_status;

    // Pass the model to HiGHS
    return_status = highs.passModel(*model);
    assert(return_status==HighsStatus::kOk);

    auto start = std::chrono::high_resolution_clock::now();
    // Solve the model
    return_status = highs.run();
    assert(return_status==HighsStatus::kOk);
    // std::cerr << "after ok" << std::endl;
    // Get the model status
    const HighsModelStatus& model_status = highs.getModelStatus();

    if (model_status != HighsModelStatus::kOptimal) {
        std::cout << "INFEASIBLE\n";
        auto finish = std::chrono::high_resolution_clock::now();
        std::cout << "time = " << std::chrono::duration<double>(finish - start).count() << '\n' << std::endl;
        return 0;
    }

    const HighsInfo& info = highs.getInfo();
    const HighsSolution& solution = highs.getSolution();
    const HighsLp& lp = highs.getLp();
    // for (int col=0; col < lp.num_col_; col++) {
    for (int col = 0; col < base_input_data.aircrafts_number * base_input_data.flights_number; ++col) {
        if (solution.col_value[col] == 1) {
            int flight_number = col / base_input_data.aircrafts_number;
            int aircraft_number = col % base_input_data.aircrafts_number;
            std::cout << "flight " << flight_number << ", aircraft " << aircraft_number << '\n';
        }
        // std::cout << "Column " << col;
        // if (info.primal_solution_status) {
        //     std::cout << "; value = " << solution.col_value[col];
        // }
        // std::cout << std::endl;
    }
    auto finish = std::chrono::high_resolution_clock::now();
    std::cout << "time = " << std::chrono::duration<double>(finish - start).count() << std::endl;
    return 0;
}
