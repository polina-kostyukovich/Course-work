#include "temperature_functions.h"

#include <cmath>

double ExponentialTemperatureFunction::operator()(int iteration) const {
    return std::exp(-iteration);
}
