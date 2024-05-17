#pragma once

class TemperatureFunction {
public:
    TemperatureFunction(double initial_temperature, double alpha);

    virtual double operator()(int iteration) = 0;

protected:
    double initial_temperature_;
    double alpha_;
};

class ExponentialTemperatureFunction : public TemperatureFunction {
public:
    explicit ExponentialTemperatureFunction(double initial_temperature, double alpha);

    double operator()(int iteration) override;
};

class LogarithmicTemperatureFunction : public TemperatureFunction {
public:
    LogarithmicTemperatureFunction(double initial_temperature, double alpha);

    double operator()(int iteration) override;
};

class GeometricTemperatureFunction : public TemperatureFunction {
public:
    GeometricTemperatureFunction(double initial_temperature, double alpha);

    double operator()(int iteration) override;

private:
    double alpha_degree_;
};
