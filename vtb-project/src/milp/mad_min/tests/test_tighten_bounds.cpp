// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#include "../tighten_bounds.hpp"

#include "test_tighten_bounds.hpp"

namespace mad_min::tests::mat
{

bool
TightenBoundsTest::test_row(MatFile& file, uint32_t row, bool /*verbose*/)
{
    bool row_ok = true;

    auto W = io::mat::load<uint16_t>(file, format("r_{}_in_W", row));
    auto rbar = io::mat::load<d_rvector>(file, format("r_{}_in_rbar", row));
    auto Gr = io::mat::load<d_cvector>(file, format("r_{}_in_Gr", row));
    auto wmin = io::mat::load<double>(file, format("r_{}_in_wmin", row));
    auto mu0 = io::mat::load<double>(file, format("r_{}_in_mu0", row));
    auto nu_lower = io::mat::load<d_cvector>(file, format("r_{}_in_nu_lower_double", row));
    auto nu_upper = io::mat::load<d_cvector>(file, format("r_{}_in_nu_upper_double", row));

    tighten_bounds(
        W,
        rbar,
        Gr,
        wmin,
        mu0,
        nu_lower,
        nu_upper);

    {
        auto nu_lower_hdf = io::mat::load<d_cvector>(file, format("r_{}_out_nu_lower_double", row));
        row_ok &= is_equal(nu_lower, nu_lower_hdf, "nu_lower");
    }

    {
        auto nu_upper_hdf = io::mat::load<d_cvector>(file, format("r_{}_out_nu_upper_double", row));
        row_ok &= is_equal(nu_upper, nu_upper_hdf, "nu_upper");
    }

    return row_ok;
}

}
