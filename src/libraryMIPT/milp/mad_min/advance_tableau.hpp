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

// advances valid simplex tableau by one iteration
// ph1 is a boolean indicating whether the second row is a virtual cost
// function as in phase 1
// T is the current value of the tableau
// B is the current basic index set
// opt is a boolean indicating whether the current value was at optimality
// ub is a boolean indicating unboundedness
// outputs T and B are the updated objects

std::tuple<bool /* opt */, bool /* ub */>
advance_tableau(
    d_matrix& T,
    u_rvector& B,
    bool ph1
);

}
