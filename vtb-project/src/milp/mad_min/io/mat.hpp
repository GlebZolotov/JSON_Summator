// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#pragma once

#include "bits/mat_device.hpp"

namespace mad_min::io::mat
{

inline
InputDevice
open(std::string_view name)
{
    return InputDevice(name);
}

template <typename T>
void
load(InputDevice& file, std::string_view variable_name, T& value);

template <typename T>
void
load(InputDevice& file, std::string_view variable_name, std::vector<T>& value);

template <typename T>
void
load(InputDevice& file, std::string_view variable_name, rvector<T>& value);

template <typename T>
void
load(InputDevice& file, std::string_view variable_name, cvector<T>& value);

template <typename T>
void
load(InputDevice& file, std::string_view variable_name, matrix<T>& value);

template <typename T>
T
load(InputDevice& file, std::string_view variable_name)
{
    T value{};
    load(file, variable_name, value);
    return value;
}

}
