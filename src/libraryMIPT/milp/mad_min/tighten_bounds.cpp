// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Roland Hildebrand <roland.hildebrand@univ-grenoble-alpes.fr>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#include "utils.hpp"

#include "tighten_bounds.hpp"

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
    d_cvector& nu_upper)
{
    auto nu_lower_old = nu_lower;
    auto nu_upper_old = nu_upper;

    // FIXME save into common parameters since rbar not changed during our life
    //
    // indm = find(commonParameters.rbar < 0);
    // indp = find(commonParameters.rbar > 0);
    auto indm = find(rbar, [](double x) { return x < 0.0; });
    auto indp = find(rbar, [](double x) { return x > 0.0; });

    while (true)
    {
        // tightening upper and lower bound by using linear constraints
        // on Gr' * nu
        //
        // gnhigh_delta = floor((1 - nu_lower'*commonParameters.Gr)./commonParameters.Gr);
        // gnlow_delta = floor((nu_upper'*commonParameters.Gr - commonParameters.wmin)./commonParameters.Gr);
        // nu_upper = min(nu_upper,nu_lower+gnhigh_delta);
        // nu_lower = max(nu_lower,nu_upper-gnlow_delta);

        const double t_lo = 1.0 - nu_lower.dot(Gr);
        const double t_up = nu_upper.dot(Gr) - wmin;

        for (size_t i = 0; i < W; ++i)
        {
            const double gnhigh_delta_i = floor(t_lo / Gr[i]);
            const double gnlow_delta_i = floor(t_up / Gr[i]);

            nu_upper[i] = std::min(nu_upper[i], nu_lower[i] + gnhigh_delta_i);
            nu_lower[i] = std::max(nu_lower[i], nu_upper[i] - gnlow_delta_i);
        }

        // tightening upper and lower bound by using linear constraints
        // on rbar * nu
        //
        // mu0_delta = commonParameters.rbar(indm)*nu_lower(indm) +
        //             commonParameters.rbar(indp)*nu_upper(indp) - commonParameters.mu0;
        double mu0_delta =
            rbar(indm).dot(nu_lower(indm)) +
            rbar(indp).dot(nu_upper(indp)) -
            mu0;

        // nu_lower(indp) = max(nu_lower(indp),nu_upper(indp)-floor(mu0_delta./(commonParameters.rbar(indp))'));
        for (auto i : indp)
        {
            const double d = floor(mu0_delta / rbar[i]);
            nu_lower[i] = std::max(nu_lower[i], nu_upper[i] - d);
        }

        // nu_upper(indm) = min(nu_upper(indm),nu_lower(indm)+floor(mu0_delta./(-commonParameters.rbar(indm))'));
        for (auto i : indm)
        {
            const double d = floor(mu0_delta / -rbar[i]);
            nu_upper[i] = std::min(nu_upper[i], nu_lower[i] + d);
        }

        // if (sum(nu_upper_old - nu_upper + nu_lower - nu_lower_old) == 0) || (min(nu_upper - nu_lower) < 0)
        if (((nu_upper_old - nu_upper + nu_lower - nu_lower_old).sum() == 0.0) ||
            ((nu_upper - nu_lower).minCoeff() < 0.0))
        {
            break;
        }

        nu_lower_old = nu_lower;
        nu_upper_old = nu_upper;
    }
}

}
