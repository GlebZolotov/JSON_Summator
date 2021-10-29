// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Roland Hildebrand <roland.hildebrand@univ-grenoble-alpes.fr>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#pragma once

#include "common.hpp"

namespace mad_min
{

struct BestIntegerSolution
{
    using Ptr = std::shared_ptr<BestIntegerSolution>;

    double upperBound;
    d_cvector bestNu;

    void
    setValues(double opt_value, const d_cvector& nu);

#if defined HDF5_TESTS
    static
    Ptr
    mat_load(std::string_view file_name, int row, std::string_view prefix = {});
#endif
};

}
