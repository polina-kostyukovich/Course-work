#include "row_wise_matrix.h"

Row::Row(const std::vector<double>& values, const std::vector<size_t>& indices)
    : non_zero_values(values), columns_indices(indices) {}

std::vector<double> RowWiseMatrix::GetValues() const {
    return values_;
}

std::vector<int> RowWiseMatrix::GetColumnIndices() const {
    return column_indices_;
}

std::vector<int> RowWiseMatrix::GetRowsStarts() const {
    return rows_starts_;
}

void RowWiseMatrix::SetColumnsNumber(size_t columns_number) {
    columns_number_ = columns_number;
}

void RowWiseMatrix::AddRow(const Row& row) {
    values_.reserve(values_.size() + row.non_zero_values.size());
    for (auto value : row.non_zero_values) {
        values_.push_back(value);
    }

    column_indices_.reserve(column_indices_.size() + row.columns_indices.size());
    for (auto index : row.columns_indices) {
        column_indices_.push_back(index);
    }

    rows_starts_.push_back(rows_starts_.back() + row.non_zero_values.size());
    rows_number_ += 1;
}

size_t RowWiseMatrix::GetRowsNumber() const {
    return rows_number_;
}

size_t RowWiseMatrix::GetColumnsNumber() const {
    return columns_number_;
}

void RowWiseMatrix::Reserve(size_t capacity) {
    values_.reserve(values_.size() + capacity);
    column_indices_.reserve(values_.size() + capacity);
}

const RowWiseMatrix& Restrictions::GetMatrix() const {
    return matrix_;
}

std::vector<double> Restrictions::GetLowBounds() const {
    return low_bounds_;
}

std::vector<double> Restrictions::GetUpBounds() const {
    return up_bounds_;
}

void Restrictions::SetColumnsNumber(size_t columns_number) {
    matrix_.SetColumnsNumber(columns_number);
}

void Restrictions::AddRow(const Row& row) {
    matrix_.AddRow(row);
    low_bounds_.push_back(row.low_bound);
    up_bounds_.push_back(row.up_bound);
}

void Restrictions::Reserve(size_t capacity) {
    matrix_.Reserve(capacity);
}
