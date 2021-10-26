// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#include "../integer_local_improve.hpp"

#include "test_integer_local_improve.hpp"

namespace mad_min::tests::mat
{

bool
IntegerLocalImproveTest::test_row(MatFile& file, uint32_t row, bool /*verbose*/)
{
    bool row_ok = true;

    auto Gr = io::mat::load<d_cvector>(file, format("r_{}_in_Gr", row));
    auto M = io::mat::load<d_matrix>(file, format("r_{}_in_M", row));
    auto mu0 = io::mat::load<double>(file, format("r_{}_in_mu0", row));
    auto nu = io::mat::load<d_cvector>(file, format("r_{}_in_nu", row));
    auto numax = io::mat::load<u_cvector>(file, format("r_{}_in_numax", row));
    auto W = io::mat::load<uint16_t>(file, format("r_{}_in_numshares", row));
    auto rbar = io::mat::load<d_rvector>(file, format("r_{}_in_rbar", row));
    auto wmin = io::mat::load<double>(file, format("r_{}_in_wmin", row));

    auto [s_cpp, value_cpp, nu_cpp] = integer_local_improve(
        rbar,
        Gr,
        M,
        W,
        numax,
        mu0,
        wmin,
        nu);

    {
        auto s_mat = (bool)io::mat::load<double>(file, format("r_{}_out_s", row));
        row_ok &= is_equal(s_cpp, s_mat, "s");
    }

    {
        auto value_mat = io::mat::load<double>(file, format("r_{}_out_value", row));
        row_ok &= is_equal(value_cpp, value_mat, "value");
    }

    {
        auto nu_mat = io::mat::load<d_cvector>(file, format("r_{}_out_nu", row));
        row_ok &= is_equal(nu_cpp, nu_mat, "nu");
    }

    return row_ok;
}

}
