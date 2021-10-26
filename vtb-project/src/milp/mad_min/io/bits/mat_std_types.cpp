// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#include "mat_load.hpp"

namespace mad_min::io::mat
{

template <typename T>
void
load(InputDevice& file, std::string_view variable_name, T& value)
{
    auto set_value = [](T& value, InputArray& array)
    {
        auto data = (T*)array.re_data();
        value = data[0];
    };

    load_impl<T, T>(file, variable_name, value, 1, 1, set_value);
}

template <typename T>
void
load(InputDevice& file, std::string_view variable_name, std::vector<T>& value)
{
    auto set_value = [](std::vector<T>& value, InputArray& array)
    {
        auto data = (T*)array.re_data();
        value = std::vector<T>(data, data + array.size());
    };

    load_impl<T, std::vector<T>>(file, variable_name, value, 1, -1, set_value);
}

#define LOAD(CPP_TYPE) \
    template void load<CPP_TYPE>(InputDevice&, std::string_view, CPP_TYPE&); \
    template void load<CPP_TYPE>(InputDevice&, std::string_view, std::vector<CPP_TYPE>&);

LOAD(int8_t);
LOAD(int16_t);
LOAD(int32_t);
LOAD(int64_t);
LOAD(uint8_t);
LOAD(uint16_t);
LOAD(uint32_t);
LOAD(uint64_t);
LOAD(float);
LOAD(double);

}
