// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Roland Hildebrand <roland.hildebrand@univ-grenoble-alpes.fr>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#pragma once

#include "parameters.hpp"

namespace mad_min
{

struct CubeReturnMaximize
{
    // s is a boolean indicating whether the problem is feasible
    bool is_feasible = false;

    /// value is the optimal value
    double value = -d_inf;

    /// nu_low,nu_upp are the lower and upper nearest integer vectors to the
    /// optimal nu
    d_cvector nu_low;
    d_cvector nu_upp;
    d_cvector nu;

    /// index is the number of the fractional element (0 if there is none)
    int index = -1;

    /// index1 is the number of index in the sorted list ind_rho_div_Gr
    /// check if problem is infeasible
    int index1 = -1;

    /// min_max is a variable in {-1,0,+1} indicating whether the minimal value
    /// of Gr'*nu or the maximal or none is hit
    int min_max = 0;
};

/// quick routine which maximizes rbar*nu over the cube {nu: nu_lower <= nu <= nu_upper} under
/// the constraints wmin <= nu'*Gr <= 1
/// dual variable is zero if unconditional maximum satisfies the constraints
/// dual variable is positive or negative in dependence on whether the lower
/// or upper constraint is violated
/// in this case the dual variable to the constraints on nu'*Gr equals -rbar_i/Gr_i for
/// some critical index i
CubeReturnMaximize
maximize_return_on_cube(
    const d_cvector& Gr,
    const d_rvector& rbar,
    const u_rvector& ind_rho_div_Gr,
    const uint16_t W,
    const double wmin,
    const d_cvector& nu_lower,
    const d_cvector& nu_upper);

inline
CubeReturnMaximize
maximize_return_on_cube(
    const Parameters& params,
    const d_cvector& nu_lower,
    const d_cvector& nu_upper)
{
    return maximize_return_on_cube(
        params.Gr,
        params.rbar,
        params.ind_rho_div_Gr,
        params.W,
        params.wmin,
        nu_lower,
        nu_upper
    );
}

}
