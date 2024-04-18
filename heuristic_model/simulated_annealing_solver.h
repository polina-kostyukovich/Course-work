#pragma once

#include <memory>
#include <vector>

#include "../help_structs/entities.h"

class SimulatedAnnealingSolver {
public:
    SimulatedAnnealingSolver(const std::shared_ptr<std::vector<Aircraft>>& aircrafts,
                             const std::shared_ptr<std::vector<Airport>>& airports,
                             int hours_in_cycle);
};
