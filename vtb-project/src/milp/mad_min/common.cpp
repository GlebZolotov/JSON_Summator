// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#include "common.hpp"

namespace mad_min
{

inline
std::chrono::system_clock::time_point
now()
{
    return std::chrono::system_clock::now();
}

Timer::Timer()
{
    restart();
}

void
Timer::restart()
{
    m_start = now();
}

double
Timer::ms() const
{
    using DurationType = std::chrono::duration<double, std::milli>;

    return std::chrono::duration_cast<DurationType>(now() - m_start).count();
}

double
Timer::s() const
{
    using DurationType = std::chrono::duration<double>;

    return std::chrono::duration_cast<DurationType>(now() - m_start).count();
}

}
