#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include <Highs.h>

#include "help_structs/data_processor.h"
#include "help_structs/data_reader.h"
#include "help_structs/entities.h"
#include "help_structs/indices_converter.h"
#include "help_structs/row_wise_matrix.h"

size_t GetNeededCapacity(size_t i_number, size_t j_number, size_t l_number, size_t k_number,
                         const std::unique_ptr<ILP::ProcessedData>& processed_data) {
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
    const std::unique_ptr<ILP::ProcessedData>& processed_data,
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
                                          const std::unique_ptr<ILP::ProcessedData>& processed_data) {
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
                * (processed_data->time_points[1] - processed_data->time_points[0] + input_data->hour_size);
            for (size_t k = 1; k < k_number - 1; ++k) {
                index = indices_converter.ConvertAIndex(l, j, k);
                target_coefficients[index] = (*input_data->airports)[l].stay_cost
                    * (processed_data->time_points[k + 1] - processed_data->time_points[k] + 2 * input_data->hour_size);
            }
            index = indices_converter.ConvertAIndex(l, j, k_number - 1);
            target_coefficients[index] = (*input_data->airports)[l].stay_cost
                * (processed_data->time_points[k_number] - processed_data->time_points[k_number - 1] + input_data->hour_size);
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

int main(int argc, char** argv) {
    const double kM = 1e10;
    std::string input_filename = "../input_data/input.json";
    std::string data_filename = "../data/testing_data.json";
    if (argc == 2) {
        auto args = GetArgs(argv[1]);
        input_filename = args[0];
        data_filename = args[1];
    } else {
        throw;
    }

    auto reader = DataReader(input_filename, data_filename);
    auto base_input_data = reader.ReadBaseData();
    auto input_data = reader.ReadData();
    // std::cout << "Read data" << std::endl;
    auto processed_data = ILP::GetProcessedData(input_data);
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
        std::cerr << 1e20;
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
    std::cerr << info.objective_function_value;
    return 0;
}
