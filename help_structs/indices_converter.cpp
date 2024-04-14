#include "indices_converter.h"

IndicesConverter::IndicesConverter(size_t flights_number, size_t aircrafts_number,
                                   size_t airports_number, size_t time_points_number)
    : upper_i_(flights_number), upper_j_(aircrafts_number), upper_l_(airports_number),
      upper_k_(time_points_number) {}

size_t IndicesConverter::ConvertXIndex(size_t i, size_t j) const {
    return upper_j_ * i + j;
}

size_t IndicesConverter::ConvertAIndex(size_t l, size_t j, size_t k) const {
    return upper_i_ * upper_j_ + upper_k_ * upper_l_ * j + upper_l_ * k + l;
}

size_t IndicesConverter::IndicesNumber() const {
    return upper_i_ * upper_j_ + upper_l_ * upper_j_ * upper_k_;
}
