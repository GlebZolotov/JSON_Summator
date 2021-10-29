// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#include "mat_array_type.hpp"

namespace mad_min::io::mat
{

#define ARRAY_TYPE(CPP_TYPE, MAT_TYPE) \
    template <> ArrayType array_type(const CPP_TYPE&) { return ArrayType::MAT_TYPE; }

ARRAY_TYPE(int8_t, Int8);
ARRAY_TYPE(int16_t, Int16);
ARRAY_TYPE(int32_t, Int32);
ARRAY_TYPE(int64_t, Int64);
ARRAY_TYPE(uint8_t, UInt8);
ARRAY_TYPE(uint16_t, UInt16);
ARRAY_TYPE(uint32_t, UInt32);
ARRAY_TYPE(uint64_t, UInt64);
ARRAY_TYPE(float, Float);
ARRAY_TYPE(double, Double);

}
