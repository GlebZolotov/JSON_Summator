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
load(InputDevice& file, std::string_view variable_name, rvector<T>& value)
{
    auto set_value = [](rvector<T>& value, InputArray& array)
    {
        value = Eigen::Map<rvector<T>>((T*)array.re_data(), array.size());
    };

    load_impl<T, rvector<T>>(file, variable_name, value, 1, -1, set_value);
}

template <typename T>
void
load(InputDevice& file, std::string_view variable_name, cvector<T>& value)
{
    auto set_value = [](cvector<T>& value, InputArray& array)
    {
        value = Eigen::Map<cvector<T>>((T*)array.re_data(), array.size());
    };

    load_impl<T, cvector<T>>(file, variable_name, value, 1, -1, set_value);
}

template <typename T>
void
load(InputDevice& file, std::string_view variable_name, matrix<T>& value)
{
    auto set_value = [](matrix<T>& value, InputArray& array)
    {
        value = Eigen::Map<matrix<T>>((T*)array.re_data(), array.dim(0), array.dim(1));
    };

    load_impl<T, matrix<T>>(file, variable_name, value, 2, -1, set_value);
}

#define LOAD(CPP_TYPE) \
    template void load(InputDevice&, std::string_view, rvector<CPP_TYPE>&); \
    template void load(InputDevice&, std::string_view, cvector<CPP_TYPE>&); \
    template void load(InputDevice&, std::string_view, matrix<CPP_TYPE>&);

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
