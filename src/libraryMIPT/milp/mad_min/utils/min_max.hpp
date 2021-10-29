// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#pragma once

#include "../common.hpp"

namespace mad_min
{

template <typename M>
auto
min(const M& m)
{
    Eigen::Index idx;
    auto value = m.minCoeff(&idx);

    return std::make_tuple(value, idx);
}

template <typename M>
auto
max(const M& m)
{
    Eigen::Index idx;
    auto value = m.maxCoeff(&idx);

    return std::make_tuple(value, idx);
}

template <typename M>
Eigen::Index
min_idx(const M& m)
{
    Eigen::Index idx;
    m.minCoeff(&idx);

    return idx;
}

template <typename M>
Eigen::Index
max_idx(const M& m)
{
    Eigen::Index idx;
    m.maxCoeff(&idx);

    return idx;
}

}
