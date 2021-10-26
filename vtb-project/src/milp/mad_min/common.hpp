// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#pragma once

#include <cassert>
#include <chrono>
#include <cmath>
#include <iostream> // FIXME remove?
#include <limits>
#include <numeric>

#include <Eigen/Dense>
#include <fmt/core.h>

using namespace fmt;

namespace mad_min
{

constexpr double d_inf = std::numeric_limits<double>::infinity();
constexpr double d_nan = std::numeric_limits<double>::signaling_NaN();

// row-vector
template <typename T>
using rvector = Eigen::Matrix<T, 1, Eigen::Dynamic>;

// column-vector
template <typename T>
using cvector = Eigen::Matrix<T, Eigen::Dynamic, 1>;

// // ColMajor (default) storage
// template <typename T>
// using matrix = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>;

// RowMajor storage, initial tests (QuickTeturnToFeasibilityHdfTest) -> ~3-4 times faster than ColMajor ?
template <typename T>
using matrix = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

using d_matrix = matrix<double>;
using d_rvector = rvector<double>;
using d_cvector = cvector<double>;

using u_rvector = rvector<uint16_t>;
using u_cvector = cvector<uint16_t>;

using i_rvector = rvector<int32_t>;
using i_cvector = cvector<int32_t>;

class Timer
{
public:
    Timer();

    void
    restart();

    double
    ms() const;

    double
    s() const;

private:
    std::chrono::time_point<std::chrono::system_clock> m_start;
};

template <typename M>
Eigen::Index
rend(const M& m)
{
    return m.rows() - 1;
}

template <typename M>
Eigen::Index
cend(const M& m)
{
    return m.cols() - 1;
}

template <typename M>
void
removeRow(M& matrix, unsigned int rowToRemove)
{
    // https://stackoverflow.com/a/21068014

    unsigned int numRows = matrix.rows() - 1;
    unsigned int numCols = matrix.cols();

    if (rowToRemove < numRows) {
        matrix.block(rowToRemove, 0, numRows - rowToRemove, numCols) =
            matrix.block(rowToRemove + 1, 0, numRows - rowToRemove, numCols).eval();
    }

    matrix.conservativeResize(numRows, numCols);
}

template <typename M>
void
removeColumn(M& matrix, unsigned int colToRemove)
{
    // https://stackoverflow.com/a/21068014

    unsigned int numRows = matrix.rows();
    unsigned int numCols = matrix.cols() - 1;

    if (colToRemove < numCols) {
        matrix.block(0, colToRemove, numRows, numCols - colToRemove) =
            matrix.block(0, colToRemove + 1, numRows, numCols - colToRemove).eval();
    }

    matrix.conservativeResize(numRows,numCols);
}

}
