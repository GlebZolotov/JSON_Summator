// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Roland Hildebrand <roland.hildebrand@univ-grenoble-alpes.fr>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#include "best_integer_solution.hpp"

namespace mad_min
{

void
BestIntegerSolution::setValues(double opt_value, const d_cvector& nu)
{
    upperBound = opt_value;

    if (std::isfinite(opt_value))
    {
        bestNu = nu;
    }
}

}
