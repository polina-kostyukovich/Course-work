#pragma once

#include <cstddef>

class IndicesConverter {
public:
    IndicesConverter(size_t flights_number, size_t aircrafts_number, size_t airports_number,
                     size_t time_points_number);

    [[nodiscard]] size_t ConvertXIndex(size_t i, size_t j) const;
    [[nodiscard]] size_t ConvertAIndex(size_t l, size_t j, size_t k) const;

    size_t IndicesNumber() const;

private:
    size_t upper_i_;
    size_t upper_j_;
    size_t upper_l_;
    size_t upper_k_;
};
