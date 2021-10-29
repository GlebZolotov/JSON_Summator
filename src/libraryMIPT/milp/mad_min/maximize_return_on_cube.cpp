// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Roland Hildebrand <roland.hildebrand@univ-grenoble-alpes.fr>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#include "utils.hpp"

#include "maximize_return_on_cube.hpp"

namespace mad_min
{

CubeReturnMaximize
maximize_return_on_cube(
    const d_cvector& Gr,
    const d_rvector& rbar,
    const u_rvector& ind_rho_div_Gr,
    const uint16_t W,
    const double wmin,
    const d_cvector& nu_lower,
    const d_cvector& nu_upper)
{
    CubeReturnMaximize out;

    const double constr_val_lower = nu_lower.dot(Gr);
    if (constr_val_lower > 1.0)
    {
        return out;
    }

    const double constr_val_upper = nu_upper.dot(Gr);
    if (constr_val_upper < wmin)
    {
        return out;
    }

    out.is_feasible = true;

    // pos_c = find(commonParameters.rbar > 0)';
    // nu_c = double(nu_lower);
    // nu_c(pos_c) = double(nu_upper(pos_c)); % nu maximizing the objective unconditionally
    out.nu_low = nu_lower;
    for (Eigen::Index i = 0; i < out.nu_low.size(); ++i)
    {
        if (rbar[i] > 0.0)
        {
            out.nu_low[i] = nu_upper[i];
        }
    }

    const double c_value = Gr.dot(out.nu_low);
    if ((c_value >= wmin) && (c_value <= 1.0)) // lambda = 0
    {
        out.value = rbar.dot(out.nu_low);
        out.nu_upp = out.nu_low;
        out.nu = out.nu_low;

        return out;
    }

    if (c_value > 1.0) // unconditional constraint value too high, lambda < 0
    {
        // scan through critical values from below
        out.nu = nu_lower;
        auto constr_val = constr_val_lower;
        while (constr_val <= 1.0)
        {
            out.index1 += 1;

            out.index = ind_rho_div_Gr[out.index1];
            out.nu[out.index] = nu_upper[out.index];
            constr_val += (nu_upper[out.index] - nu_lower[out.index]) * Gr[out.index];
        }
        out.nu[out.index] = nu_upper[out.index] - (constr_val - 1.0) / Gr[out.index];
        out.min_max = 1;
    }
    else // unconditional constraint value too low, lambda > 0
    {
        // scan through critical values from above
        out.index1 = W - 1;
        out.nu = nu_upper;
        auto constr_val = constr_val_upper;
        while (constr_val >= wmin)
        {
            out.index = ind_rho_div_Gr[out.index1];
            out.nu[out.index] = nu_lower[out.index];
            constr_val -= (nu_upper[out.index] - nu_lower[out.index]) * Gr[out.index];

            out.index1 -= 1;
        }
        out.nu[out.index] = nu_lower[out.index] + (wmin - constr_val) / Gr[out.index];
        out.min_max = -1;
    }
    out.value = rbar.dot(out.nu);

    out.nu_low = out.nu;
    out.nu_low[out.index] = floor(out.nu_low[out.index]);

    out.nu_upp = out.nu_low;
    out.nu_upp[out.index] += 1.0;

    return out;
}

}
