#pragma once

#include <vector>

struct Row {
    std::vector<double> non_zero_values;
    std::vector<size_t> columns_indices;
    double low_bound{-1.0e30};
    double up_bound{1.0e30};

    Row() = default;
    Row(const std::vector<double>& values, const std::vector<size_t>& indices);
};

class RowWiseMatrix {
public:
    [[nodiscard]] std::vector<double> GetValues() const;
    [[nodiscard]] std::vector<int> GetColumnIndices() const;
    [[nodiscard]] std::vector<int> GetRowsStarts() const;

    [[nodiscard]] size_t GetRowsNumber() const;
    [[nodiscard]] size_t GetColumnsNumber() const;

    void SetColumnsNumber(size_t columns_number);

    void AddRow(const Row& row);

    void Reserve(size_t capacity);

private:
    std::vector<double> values_;
    std::vector<int> column_indices_;
    std::vector<int> rows_starts_ = {0};
    size_t rows_number_{0};
    size_t columns_number_{0};
};

class Restrictions {
public:
    [[nodiscard]] const RowWiseMatrix& GetMatrix() const;
    [[nodiscard]] std::vector<double> GetLowBounds() const;
    [[nodiscard]] std::vector<double> GetUpBounds() const;

    void SetColumnsNumber(size_t columns_number);
    void AddRow(const Row& row);

    void Reserve(size_t capacity);

private:
    RowWiseMatrix matrix_;
    std::vector<double> low_bounds_;
    std::vector<double> up_bounds_;
};
