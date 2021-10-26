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

template <typename V, typename C>
std::vector<Eigen::Index>
find(const V& vec, C comp)
{
    std::vector<Eigen::Index> out;
    for (Eigen::Index i = 0; i < vec.size(); ++i)
    {
        if (comp(vec[i]))
        {
            out.push_back(i);
        }
    }

    return out;
}

template <typename V, typename C>
void
find(const V& vec, C comp, std::vector<Eigen::Index>& result)
{
    result.clear();
    for (Eigen::Index i = 0; i < vec.size(); ++i)
    {
        if (comp(vec[i]))
        {
            result.push_back(i);
        }
    }
}

template <typename V, typename C>
Eigen::Index
find1(const V& vec, C comp)
{
    for (Eigen::Index i = 0; i < vec.size(); ++i)
    {
        if (comp(vec[i]))
        {
            return i;
        }
    }

    return -1;
}

template <typename T>
class equal_to
{
public:
    equal_to(T value) : m_value(value) {}

    bool
    operator()(T value) const
    {
        return value == m_value;
    }

private:
    T m_value;
};

}
