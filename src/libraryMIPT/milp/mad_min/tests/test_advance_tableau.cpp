// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#include "../advance_tableau.hpp"

#include "test_advance_tableau.hpp"

namespace mad_min::tests::mat
{

bool
AdvanceTableauTest::test_row(MatFile& file, uint32_t row, bool /*verbose*/)
{
    bool row_ok = true;

    auto ph1 = (bool)io::mat::load<double>(file, format("r_{}_in_ph1", row));
    auto Tab = io::mat::load<d_matrix>(file, format("r_{}_in_T", row));
    auto Bas = io::mat::load<u_rvector>(file, format("r_{}_in_B", row));
    Bas.array() -= 1; // FIX indexing, Matlab is 1-based

    auto [opt, ub] = advance_tableau(Tab, Bas, ph1);

    {
        auto Tab_hdf = io::mat::load<d_matrix>(file, format("r_{}_out_T", row));
        row_ok &= is_equal(Tab, Tab_hdf, "Tab");
    }

    {
        auto Bas_hdf = io::mat::load<u_rvector>(file, format("r_{}_out_B", row));
        Bas_hdf.array() -= 1; // FIX indexing, Matlab is 1-based
        row_ok &= is_equal(Bas, Bas_hdf, "Bas");
    }

    {
        auto opt_hdf = (bool)io::mat::load<double>(file, format("r_{}_out_opt", row));
        row_ok &= is_equal(opt, opt_hdf, "opt");
    }

    {
        auto ub_hdf = (bool)io::mat::load<double>(file, format("r_{}_out_ub", row));
        row_ok &= is_equal(ub, ub_hdf, "ub");
    }

    return row_ok;
}

}

namespace mad_min::tests::hdf
{

bool
AdvanceTableauTest::test_row(const H5Easy::File& file, uint32_t row, bool /*verbose*/)
{
    bool row_ok = true;

    auto ph1 = (bool)H5Easy::load<double>(file, format("/{}/in/ph1", row));
    auto Tab = H5Easy::load<d_matrix>(file, format("/{}/in/T", row));
    auto Bas = H5Easy::load<u_rvector>(file, format("/{}/in/B", row));
    Bas.array() -= 1; // FIX indexing, Matlab is 1-based

    auto [opt, ub] = advance_tableau(Tab, Bas, ph1);

    {
        auto Tab_hdf = H5Easy::load<d_matrix>(file, format("/{}/out/T", row));
        row_ok &= is_equal(Tab, Tab_hdf, "Tab");
    }

    {
        auto Bas_hdf = H5Easy::load<u_rvector>(file, format("/{}/out/B", row));
        Bas_hdf.array() -= 1; // FIX indexing, Matlab is 1-based
        row_ok &= is_equal(Bas, Bas_hdf, "Bas");
    }

    {
        auto opt_hdf = (bool)H5Easy::load<double>(file, format("/{}/out/opt", row));
        row_ok &= is_equal(opt, opt_hdf, "opt");
    }

    {
        auto ub_hdf = (bool)H5Easy::load<double>(file, format("/{}/out/ub", row));
        row_ok &= is_equal(ub, ub_hdf, "ub");
    }

    return row_ok;
}

}
