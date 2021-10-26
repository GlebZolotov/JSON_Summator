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

// adapts a simplex table to a change in the right-hand side of a linear
// equation system with m equations and n variables
// Tab in R^((m+1) x (n+1)) is the table to be corrected, Bas in R^m the basic
// set
// delta_slack in R^n is the column vector of additive changes in the variables
// negative entries mean the corresponding constraints are tightened,
// positive entries mean the constraints are relaxed
// s is a boolean indicating whether a feasible point has been found with the new
// bounds
// we introduce two auxiliary variables y,z >= 0, y+z = 1
// at the current point y = 0, z = 1, z added to basic set
// at the sought feasible point y = 1, z = 0

bool
return_table_to_feasibility(
    const d_cvector& delta_slack,
    d_matrix& Tab,
    u_rvector& Bas,
    bool verbose = false);

}
