#include "temperature_functions.h"

#include <cmath>

TemperatureFunction::TemperatureFunction(double initial_temperature, double alpha)
    : initial_temperature_(initial_temperature), alpha_(alpha) {}

ExponentialTemperatureFunction::ExponentialTemperatureFunction(double initial_temperature, double alpha)
    : TemperatureFunction(initial_temperature, alpha) {}

double ExponentialTemperatureFunction::operator()(int iteration) {
    return initial_temperature_ * std::exp(-alpha_ * iteration);
}

LogarithmicTemperatureFunction::LogarithmicTemperatureFunction(double initial_temperature, double alpha)
    : TemperatureFunction(initial_temperature, alpha) {}

double LogarithmicTemperatureFunction::operator()(int iteration) {
    return alpha_ * initial_temperature_ / std::log(2 + iteration);
}

GeometricTemperatureFunction::GeometricTemperatureFunction(double initial_temperature, double alpha)
    : TemperatureFunction(initial_temperature, alpha), alpha_degree_(1) {}

double GeometricTemperatureFunction::operator()(int iteration) {
    double result = alpha_degree_ * initial_temperature_;
    alpha_degree_ *= alpha_;
    return result;
}
