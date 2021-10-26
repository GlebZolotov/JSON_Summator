// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#include "../maximize_return_on_cube.hpp"

#include "test_maximize_return_on_cube.hpp"

namespace mad_min::tests::mat
{

bool
MaximizeReturnOnCubeTest::test_row(MatFile& file, uint32_t row, bool /*verbose*/)
{
    bool row_ok = true;

    auto Gr = io::mat::load<d_cvector>(file, format("r_{}_in_Gr", row));
    auto rbar = io::mat::load<d_rvector>(file, format("r_{}_in_rbar", row));
    auto ind_rho_div_Gr = io::mat::load<u_rvector>(file, format("r_{}_in_ind_rho_div_Gr", row));
    auto W = io::mat::load<uint16_t>(file, format("r_{}_in_W", row));
    auto wmin = io::mat::load<double>(file, format("r_{}_in_wmin", row));
    auto nu_lower = io::mat::load<d_cvector>(file, format("r_{}_in_nu_lower", row));
    auto nu_upper = io::mat::load<d_cvector>(file, format("r_{}_in_nu_upper", row));

    ind_rho_div_Gr.array() -= 1; // FIX indexing, Matlab is 1-based

    auto r = maximize_return_on_cube(Gr, rbar, ind_rho_div_Gr, W, wmin, nu_lower, nu_upper);

    {
        auto s_hdf = (bool)io::mat::load<double>(file, format("r_{}_out_s", row));
        row_ok &= is_equal(r.is_feasible, s_hdf, "s");
    }

    {
        auto value_hdf = io::mat::load<double>(file, format("r_{}_out_value", row));
        row_ok &= is_equal(r.value, value_hdf, "value");
    }

    if (r.is_feasible == false)
    {
        return row_ok;
    }

    {
        auto nu_low_hdf = io::mat::load<d_cvector>(file, format("r_{}_out_nu_low", row));
        row_ok &= is_equal(r.nu_low, nu_low_hdf, "nu_low");
    }

    {
        auto nu_upp_hdf = io::mat::load<d_cvector>(file, format("r_{}_out_nu_upp", row));
        row_ok &= is_equal(r.nu_upp, nu_upp_hdf, "nu_upp");
    }

    {
        auto nu_hdf = io::mat::load<d_cvector>(file, format("r_{}_out_nu", row));
        row_ok &= is_equal(r.nu, nu_hdf, "nu");
    }

    {
        auto index_hdf = (int)io::mat::load<int16_t>(file, format("r_{}_out_index", row));
        --index_hdf; // FIX indexing, Matlab is 1-based
        row_ok &= is_equal(r.index, index_hdf, "index");
    }

    {
        auto index1_hdf = (int)io::mat::load<int16_t>(file, format("r_{}_out_index1", row));
        --index1_hdf; // FIX indexing, Matlab is 1-based
        row_ok &= is_equal(r.index1, index1_hdf, "index1");
    }

    {
        auto min_max_hdf = (int)io::mat::load<double>(file, format("r_{}_out_min_max", row));
        row_ok &= is_equal(r.min_max, min_max_hdf, "min_max");
    }

    return row_ok;
}

}
