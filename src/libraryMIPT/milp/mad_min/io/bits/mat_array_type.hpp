// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#pragma once

#include <fmt/format.h>

namespace mad_min::io::mat
{

enum class ArrayType
{
    Int8,
    Int16,
    Int32,
    Int64,

    UInt8,
    UInt16,
    UInt32,
    UInt64,

    Float,
    Double,

    Unsupported
};

template <typename T>
ArrayType
array_type(const T&);

}

namespace fmt
{

template <>
struct formatter<mad_min::io::mat::ArrayType>
{
    using ArrayType = mad_min::io::mat::ArrayType;

    template <typename ParseContext>
    constexpr auto
    parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto
    format(ArrayType type, FormatContext& ctx)
    {
        switch (type)
        {
            case ArrayType::Int8:
                return format_to(ctx.out(), "Int8");

            case ArrayType::UInt8:
                return format_to(ctx.out(), "UInt8");

            case ArrayType::Int16:
                return format_to(ctx.out(), "Int16");

            case ArrayType::UInt16:
                return format_to(ctx.out(), "UInt16");

            case ArrayType::Int32:
                return format_to(ctx.out(), "Int32");

            case ArrayType::UInt32:
                return format_to(ctx.out(), "UInt32");

            case ArrayType::Int64:
                return format_to(ctx.out(), "Int64");

            case ArrayType::UInt64:
                return format_to(ctx.out(), "UInt64");

            case ArrayType::Float:
                return format_to(ctx.out(), "Float");

            case ArrayType::Double:
                return format_to(ctx.out(), "Double");

            case ArrayType::Unsupported:
                break;
        };

        return format_to(ctx.out(), "Unsupported");
    }
};

}
