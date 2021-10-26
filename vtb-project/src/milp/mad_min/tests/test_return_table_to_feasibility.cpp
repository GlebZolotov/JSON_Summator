// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#include "../return_table_to_feasibility.hpp"

#include "test_return_table_to_feasibility.hpp"

namespace mad_min::tests::mat
{

bool
ReturnTableToFeasibilityTest::test_row(MatFile& file, uint32_t row, bool verbose)
{
    bool row_ok = true;

    auto delta_slack = io::mat::load<d_cvector>(file, format("r_{}_in_delta_slack", row));
    auto Tab = io::mat::load<d_matrix>(file, format("r_{}_in_Tab", row));
    auto Bas = io::mat::load<u_rvector>(file, format("r_{}_in_Bas", row));
    Bas.array() -= 1; // FIX indexing, Matlab is 1-based

    auto s = return_table_to_feasibility(delta_slack, Tab, Bas, verbose);

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
