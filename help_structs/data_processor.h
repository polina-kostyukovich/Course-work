#pragma once

#include <iostream>
#include <memory>
#include <vector>

#include "entities.h"

namespace ILP {

struct ProcessedData {
    std::vector<int> time_points;
    std::vector<size_t> departure_times_indices;
    std::vector<size_t> arrival_times_indices;
    std::vector<std::vector<int>> is_flight_flying;
    std::vector<std::vector<int>> departure_airports_indicator;
    std::vector<std::vector<int>> arrival_airports_indicator;

    void Print() const;
};

std::unique_ptr<ProcessedData> GetProcessedData(const std::unique_ptr<InputData>& input_data);

std::vector<int> GetTimePointsArrayForILP(const std::unique_ptr<InputData>& input_data);

}

namespace heuristic {

std::vector<int> GetTimePointsArrayForHeuristic(const std::shared_ptr<std::vector<Flight>>& flights,
                                                int hour_size);

void ReplaceTimePointsWithIndices(std::shared_ptr<std::vector<Flight>>& flights,
                                  const std::vector<int>& time_points,
                                  int hour_size);

}
