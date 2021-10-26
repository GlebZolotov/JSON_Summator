// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#include "../advance_tableau.hpp"
#include "../node_build_simplex_table.hpp"

#include "test_node_build_simplex_table.hpp"

using index_vector = std::vector<uint16_t>;

namespace mad_min::tests::mat
{

bool
NodeBuildSimplexTable::test_row(MatFile& file, uint32_t row, bool /*verbose*/)
{
    auto fix_index = [](index_vector& v)
    {
        // FIX indexing, Matlab is 1-based
        for (auto& i : v)
        {
            --i;
        }
    };

    constexpr bool debug = false;
    bool row_ok = true;

    auto params = Parameters::mat_load(file.name(), row);
    auto LowerNuBound = io::mat::load<d_cvector>(file, format("r_{}_LowerNuBound", row));
    auto UpperNuBound = io::mat::load<d_cvector>(file, format("r_{}_UpperNuBound", row));

    SimplexTableBuilder builder;
    auto s = builder.build(*params, LowerNuBound, UpperNuBound);

    {
        auto s_hdf = (bool)io::mat::load<double>(file, format("r_{}_s", row));
        row_ok &= is_equal(s, s_hdf, "s");
    }

    if (debug)
    {
        {
            auto determined_nu = io::mat::load<index_vector>(file, format("r_{}_determined_nu", row));
            fix_index(determined_nu);
            row_ok &= is_equal(builder.determined_nu, determined_nu, "determined_nu");
        }

        {
            auto non_determined_nu = io::mat::load<index_vector>(file, format("r_{}_non_determined_nu", row));
            fix_index(non_determined_nu);
            row_ok &= is_equal(builder.non_determined_nu, non_determined_nu, "non_determined_nu");
        }

        {
            auto pos_a = io::mat::load<index_vector>(file, format("r_{}_pos_a", row));
            fix_index(pos_a);
            row_ok &= is_equal(builder.pos_a, pos_a, "pos_a");
        }

        {
            auto pos_b = io::mat::load<index_vector>(file, format("r_{}_pos_b", row));
            fix_index(pos_b);
            row_ok &= is_equal(builder.pos_b, pos_b, "pos_b");
        }

        {
            auto ind_sab = io::mat::load<index_vector>(file, format("r_{}_ind_sab", row));
            fix_index(ind_sab);
            row_ok &= is_equal(builder.ind_sab, ind_sab, "ind_sab");
        }

        {
            auto low_ind = io::mat::load<index_vector>(file, format("r_{}_low_ind", row));
            fix_index(low_ind);
            row_ok &= is_equal(builder.low_ind, low_ind, "low_ind");
        }

        {
            auto upp_ind = io::mat::load<index_vector>(file, format("r_{}_upp_ind", row));
            fix_index(upp_ind);
            row_ok &= is_equal(builder.upp_ind, upp_ind, "upp_ind");
        }

        {
            auto A = io::mat::load<d_matrix>(file, format("r_{}_A", row));
            row_ok &= is_equal(builder.A, A, "A");
        }

        {
            auto B = io::mat::load<d_matrix>(file, format("r_{}_B", row));
            row_ok &= is_equal(builder.B, B, "B");
        }
    }

    {
        auto Tab = io::mat::load<d_matrix>(file, format("r_{}_Tab", row));
        row_ok &= is_equal(builder.Tab, Tab, "Tab");
    }

    {
        auto Bas = io::mat::load<u_rvector>(file, format("r_{}_Bas", row));
        Bas.array() -= 1; // FIX indexing, Matlab is 1-based
        row_ok &= is_equal(builder.Bas, Bas, "Bas");
    }

    return row_ok;
}

}

namespace mad_min::tests::hdf
{

bool
NodeBuildSimplexTable::test_row(const H5Easy::File& file, uint32_t row, bool /*verbose*/)
{
    auto fix_index = [](index_vector& v)
    {
        // FIX indexing, Matlab is 1-based
        for (auto& i : v)
        {
            --i;
        }
    };

    constexpr bool debug = false;
    bool row_ok = true;

    auto params = Parameters::h5_load(file.getName(), row);
    auto LowerNuBound = H5Easy::load<d_cvector>(file, format("r_{}_LowerNuBound", row));
    auto UpperNuBound = H5Easy::load<d_cvector>(file, format("r_{}_UpperNuBound", row));

    SimplexTableBuilder builder;
    auto s = builder.build(*params, LowerNuBound, UpperNuBound);

    {
        auto s_hdf = (bool)H5Easy::load<double>(file, format("r_{}_s", row));
        row_ok &= is_equal(s, s_hdf, "s");
    }

    if (debug)
    {
        {
            auto determined_nu = H5Easy::load<index_vector>(file, format("r_{}_determined_nu", row));
            fix_index(determined_nu);
            row_ok &= is_equal(builder.determined_nu, determined_nu, "determined_nu");
        }

        {
            auto non_determined_nu = H5Easy::load<index_vector>(file, format("r_{}_non_determined_nu", row));
            fix_index(non_determined_nu);
            row_ok &= is_equal(builder.non_determined_nu, non_determined_nu, "non_determined_nu");
        }

        {
            auto pos_a = H5Easy::load<index_vector>(file, format("r_{}_pos_a", row));
            fix_index(pos_a);
            row_ok &= is_equal(builder.pos_a, pos_a, "pos_a");
        }

        {
            auto pos_b = H5Easy::load<index_vector>(file, format("r_{}_pos_b", row));
            fix_index(pos_b);
            row_ok &= is_equal(builder.pos_b, pos_b, "pos_b");
        }

        {
            auto ind_sab = H5Easy::load<index_vector>(file, format("r_{}_ind_sab", row));
            fix_index(ind_sab);
            row_ok &= is_equal(builder.ind_sab, ind_sab, "ind_sab");
        }

        {
            auto low_ind = H5Easy::load<index_vector>(file, format("r_{}_low_ind", row));
            fix_index(low_ind);
            row_ok &= is_equal(builder.low_ind, low_ind, "low_ind");
        }

        {
            auto upp_ind = H5Easy::load<index_vector>(file, format("r_{}_upp_ind", row));
            fix_index(upp_ind);
            row_ok &= is_equal(builder.upp_ind, upp_ind, "upp_ind");
        }

        {
            auto A = H5Easy::load<d_matrix>(file, format("r_{}_A", row));
            row_ok &= is_equal(builder.A, A, "A");
        }

        {
            auto B = H5Easy::load<d_matrix>(file, format("r_{}_B", row));
            row_ok &= is_equal(builder.B, B, "B");
        }
    }

    {
        auto Tab = H5Easy::load<d_matrix>(file, format("r_{}_Tab", row));
        row_ok &= is_equal(builder.Tab, Tab, "Tab");
    }

    {
        auto Bas = H5Easy::load<u_rvector>(file, format("r_{}_Bas", row));
        Bas.array() -= 1; // FIX indexing, Matlab is 1-based
        row_ok &= is_equal(builder.Bas, Bas, "Bas");
    }

    return row_ok;
}

}
