#include "data_processor.h"

#include <algorithm>

namespace ILP {

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

std::vector<int> GetTimePointsArrayForILP(const std::unique_ptr<InputData>& input_data) {
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

void ProcessedData::Print() const {
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

std::unique_ptr<ProcessedData> GetProcessedData(const std::unique_ptr<InputData>& input_data) {
    auto processed_data = std::make_unique<ProcessedData>();
    processed_data->time_points = GetTimePointsArrayForILP(input_data);
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

}

namespace heuristic {

std::vector<int> GetTimePointsArrayForHeuristic(const std::shared_ptr<std::vector<Flight>>& flights) {
    std::vector<int> time_points;
    time_points.reserve(2 * flights->size());
    for (auto& flight : (*flights)) {
        time_points.push_back(flight.departure_time - 1);
        time_points.push_back(flight.arrival_time + 1);
    }

    std::sort(time_points.begin(), time_points.end());
    auto new_end = std::unique(time_points.begin(), time_points.end());
    time_points.resize(new_end - time_points.begin());
    return time_points;
}

void ReplaceTimePointsWithIndices(std::shared_ptr<std::vector<Flight>>& flights,
                                  const std::vector<int>& time_points) {
    for (auto& flight : *flights) {
        auto departure_index = std::lower_bound(time_points.begin(),
                                                time_points.end(),
                                                flight.departure_time - 1) - time_points.begin();
        auto arrival_index = std::lower_bound(time_points.begin(),
                                              time_points.end(),
                                              flight.arrival_time + 1) - time_points.begin();
        flight.departure_time = departure_index;
        flight.arrival_time = arrival_index;
    }
}

}
