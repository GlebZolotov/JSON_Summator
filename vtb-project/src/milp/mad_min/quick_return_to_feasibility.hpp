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

// in the linear system corresponding to the simplex table Tab the basic
// variable j, corresponding to row q, has been changed by delta
// this can be corrected by adjusting the right-hand side of equation q
// if the table becomes infeasible, then a phase 1 like optimization is
// performed to return slack j to zero
// s is a boolean indicating whether a feasible point has been reached

bool
quick_return_to_feasibility(
    double delta,
    uint16_t j,
    uint16_t q,
    d_matrix& Tab,
    u_rvector& Bas,
    bool verbose = false);

}
