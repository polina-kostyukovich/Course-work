#pragma once

struct TemperatureFunction {
    virtual double operator()(int iteration) const = 0;
};

struct ExponentialTemperatureFunction : public TemperatureFunction {
    double operator()(int iteration) const override;
};
