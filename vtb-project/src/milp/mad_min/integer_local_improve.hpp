// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Roland Hildebrand <roland.hildebrand@univ-grenoble-alpes.fr>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#pragma once

#include "parameters.hpp"

// part of the MAD min problem
// nu is an integer vector
// function checks its feasibility
// if it is feasible, greedy local descent is tried by changing entries by
// +-1
// if the solution cannot be further improved, the corresponding variables
// are given back
// value is the minimal achieved objective value
// nu are the solution variables
// s is a boolean indicating whether a feasible solution has been found
// check feasibility

namespace mad_min
{

std::tuple<bool /*s*/, double /*value*/, d_cvector /*nu*/>
integer_local_improve(
    const d_rvector& rbar,
    const d_cvector& Gr,
    const d_matrix& M,
    uint16_t W,
    const u_cvector& numax,
    double mu0,
    double wmin,
    const d_cvector& nu);

inline
std::tuple<bool /*s*/, double /*value*/, d_cvector /*nu*/>
integer_local_improve(const Parameters& params, const d_cvector& nu)
{
    return integer_local_improve(
            params.rbar,
            params.Gr,
            params.M,
            params.W,
            params.numax,
            params.mu0,
            params.wmin,
            nu);
}

}
