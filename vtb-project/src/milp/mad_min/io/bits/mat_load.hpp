// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#pragma once

#include "mat_array.hpp"

namespace mad_min::io::mat
{

template <typename T, typename V>
void
load_impl(
    InputDevice& file,
    std::string_view variable_name,
    V& value,
    size_t rank,
    int size,
    std::function<void(V&,InputArray&)> set_value)
{
    const auto value_type = array_type(T{});
    if (value_type == ArrayType::Unsupported)
    {
        // TODO add type name
        // https://stackoverflow.com/questions/4484982/how-to-convert-typename-t-to-string-in-c
        throw Error("Unsupported type of value");
    }

    InputArray array(file, variable_name);
    if (array.type() == ArrayType::Unsupported)
    {
        throw Error("Unsupported type of Matlab variable '{}'", variable_name);
    }

    if (value_type != array.type())
    {
        throw Error("Different types: {} (value) != {} (Matlab variable '{}')",
                    value_type, array.type(), variable_name);
    }

    if ((array.rank() != rank) && (array.size() > 0))
    {
        throw Error("Wrong rank ({}) of Matlab variable '{}'", array.rank(), variable_name);
    }

    if ((size > 0) && (array.size() != (size_t)size))
    {
        throw Error("Wrong size ({}) of Matlab variable '{}'", array.size(), variable_name);
    }

    array.read_data();
    set_value(value, array);
}

}
