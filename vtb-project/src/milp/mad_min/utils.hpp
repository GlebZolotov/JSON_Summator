// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#pragma once

#include <algorithm>
#include <optional>

#include "utils/find.hpp"
#include "utils/min_max.hpp"
#include "utils/string.hpp"

namespace mad_min
{

template <typename T>
class reverse
{
public:
    explicit
    reverse(T& iterable)
        : _iterable{iterable}
    {
    }

    explicit
    reverse(T&& iterable)
        : _iterable{ std::forward<T>(iterable) }
    {
    }

    auto
    begin()
    {
        return std::rbegin(_iterable);
    }

    auto
    end()
    {
        return std::rend(_iterable);
    }

private:
//     T& _iterable;
    T _iterable;
};

}
