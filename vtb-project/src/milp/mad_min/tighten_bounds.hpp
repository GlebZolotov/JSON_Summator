// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Roland Hildebrand <roland.hildebrand@univ-grenoble-alpes.fr>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#pragma once

#include "parameters.hpp"

// presolve for MAD min problem
// tightens the box bounds for the MAD min problem
// uses the three linear inequality constraints with non-integer coefficients
// the coefficients are taken from the commonParameters object
// the bounds are integer vectors
// tightening is repeated until no further progress is achieved
// tightening the bounds using this (single inequality) way
// can later be replaced by LPs minimizing and maximizing
// individual integer variables

namespace mad_min
{

void
tighten_bounds(
    const uint16_t W,
    const d_rvector& rbar,
    const d_cvector& Gr,
    const double wmin,
    const double mu0,
    d_cvector& nu_lower,
    d_cvector& nu_upper);

inline
void
tighten_bounds(
    const Parameters& params,
    d_cvector& nu_lower,
    d_cvector& nu_upper)
{
    tighten_bounds(
        params.W,
        params.rbar,
        params.Gr,
        params.wmin,
        params.mu0,
        nu_lower,
        nu_upper
    );
}

}
