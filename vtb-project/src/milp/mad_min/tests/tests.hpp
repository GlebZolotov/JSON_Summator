// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#pragma once

// FIXME remove when migrate to MAT
#include <highfive/H5Easy.hpp>

#include "../io/mat.hpp"

namespace mad_min::tests::mat
{

class MatTest
{
public:
    using MatFile = io::mat::InputDevice;

    virtual
    bool
    run(const std::string& file_name, bool verbose = false) final;

protected:
    virtual
    bool
    test_row(MatFile& file, uint32_t row, bool verbose) = 0;
};

}

namespace mad_min::tests::hdf
{

class HdfTest
{
public:
    virtual
    bool
    run(const std::string& hdf_file_name, bool verbose = false) final;

protected:
    virtual
    bool
    test_row(const H5Easy::File& file, uint32_t row, bool verbose) = 0;
};

}

namespace mad_min::tests
{

template <int _Rows, int _Cols, int _Options, int _MaxRows, int _MaxCols>
bool
is_equal(
    const Eigen::Matrix<double, _Rows, _Cols, _Options, _MaxRows, _MaxCols>& m1,
    const Eigen::Matrix<double, _Rows, _Cols, _Options, _MaxRows, _MaxCols>& m2,
    const std::string_view name,
    const double eps = 1e-6)
{
    if (m1.size() != m2.size())
    {
        print("\n    {} size is differ: {} x {} != {} x {}",
              name,
              m1.rows(), m1.cols(),
              m2.rows(), m2.cols());
        return false;
    }

    auto delta = (m1 - m2).norm();
    if ((std::isfinite(delta) == false) || (delta >= eps))
    {
        print("\n    {} = {:e}", name, delta);
        return false;
    }

    return true;
}

template <int _Rows, int _Cols, int _Options, int _MaxRows, int _MaxCols>
bool
is_equal(
    const Eigen::Matrix<uint16_t, _Rows, _Cols, _Options, _MaxRows, _MaxCols>& m1,
    const Eigen::Matrix<uint16_t, _Rows, _Cols, _Options, _MaxRows, _MaxCols>& m2,
    const std::string_view name)
{
    if (m1.size() != m2.size())
    {
        print("\n    {} size is differ: {} x {} != {} x {}",
              name,
              m1.rows(), m1.cols(),
              m2.rows(), m2.cols());
        return false;
    }

    auto delta = (m1.template cast<double>() - m2.template cast<double>()).template lpNorm<1>();
    if (delta > 0)
    {
        print("\n    {} = {}", name, delta);
        return false;
    }

    return true;
}

inline
bool
is_equal(
    double v1,
    double v2,
    const std::string_view name,
    const double eps = 1e-6)
{
    if (v1 == v2)
    {
        return true;
    }

    if ((std::isfinite(v1) && std::isfinite(v2)) == false)
    {
        print("\n    {}: {} != {}", name, v1, v2);
        return false;
    }

    const double delta = fabs(v1 - v2);
    if ((std::isfinite(delta) == false) || (delta >= eps))
    {
        print("\n    {} = {:e}", name, delta);
        return false;
    }

    return true;
}

template <typename T>
bool
is_equal(
    const std::vector<T>& v1,
    const std::vector<T>& v2,
    const std::string_view name,
    const double eps = 1e-6)
{
    if (v1.size() != v2.size())
    {
        print("\n    {} size is differ: {} != {}", name, v1.size(), v2.size());
        return false;
    }

    const auto m1 = Eigen::Map<const rvector<T>>(v1.data(), v1.size());
    const auto m2 = Eigen::Map<const rvector<T>>(v2.data(), v2.size());

    auto delta = (m1.template cast<double>() - m2.template cast<double>()).norm();
    if ((std::isfinite(delta) == false) || (delta >= eps))
    {
        print("\n    {} delta nrm2 = {:g}", name, delta);
        return false;
    }

    return true;
}

template <typename T>
bool
is_equal(
    const T& v1,
    const T& v2,
    const std::string_view name)
{
    if (v1 != v2)
    {
        print("\n    {}: {} != {}", name, v1, v2);
        return false;
    }

    return true;
}

}
