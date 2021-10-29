// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#include "../quick_return_to_feasibility.hpp"

#include "test_quick_return_to_feasibility.hpp"

namespace mad_min::tests::mat
{

bool
QuickReturnToFeasibilityTest::test_row(MatFile& file, uint32_t row, bool verbose)
{
    bool row_ok = true;

    auto delta = io::mat::load<double>(file, format("r_{}_in_delta", row));
    auto j = (uint16_t)io::mat::load<double>(file, format("r_{}_in_j", row));
    auto q = (uint16_t)io::mat::load<double>(file, format("r_{}_in_q", row));
    auto Tab = io::mat::load<d_matrix>(file, format("r_{}_in_Tab", row));
    auto Bas = io::mat::load<u_rvector>(file, format("r_{}_in_Bas", row));

    // FIX indexing, Matlab is 1-based
    j -= 1;
    q -= 1;
    Bas.array() -= 1;

    auto s = quick_return_to_feasibility(delta, j, q, Tab, Bas, verbose);

    {
        auto s_hdf = (bool)io::mat::load<double>(file, format("r_{}_out_s", row));
        row_ok &= is_equal(s, s_hdf, "s");
    }

    {
        auto Tab_hdf = io::mat::load<d_matrix>(file, format("r_{}_out_Tab", row));
        row_ok &= is_equal(Tab, Tab_hdf, "Tab");
    }

    {
        auto Bas_hdf = io::mat::load<u_rvector>(file, format("r_{}_out_Bas", row));
        Bas_hdf.array() -= 1; // FIX indexing, Matlab is 1-based
        row_ok &= is_equal(Bas, Bas_hdf, "Bas");
    }

    return row_ok;
}

}
