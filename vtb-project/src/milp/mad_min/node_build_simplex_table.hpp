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

#if defined HDF5_TESTS
namespace tests::hdf
{
    class NodeBuildSimplexTable;
}
namespace tests::mat
{
    class NodeBuildSimplexTable;
}
#endif

struct SimplexTableBuilder
{
    d_matrix Tab;
    u_rvector Bas;
    std::vector<uint16_t> non_determined_nu;

    // build a simplex table for the MAD min LP relaxations from a feasible solution nu
    // the feasible solution is obtained by the maximum on cube routine
    // s (return) is a boolean indicating whether the LP is feasible
    // initializes also the non_determined_nu index list correctly
    // this function is to be used if the simplex table in the parent node is not
    // appropriate or absent

    bool
    build(
        const Parameters& params,
        const d_cvector& LowerNuBound,
        const d_cvector& UpperNuBound
    );

private:
    void
    intersect(const Parameters& params, int index1);

    std::vector<uint16_t> determined_nu;
    std::vector<uint16_t> pos_a;
    std::vector<uint16_t> pos_b;
    std::vector<uint16_t> ind_sab;

    std::vector<uint16_t> low_ind;
    std::vector<uint16_t> upp_ind;
    std::vector<uint8_t> temp;

    d_matrix A;
    d_matrix B;

#if defined HDF5_TESTS
    friend class tests::hdf::NodeBuildSimplexTable;
    friend class tests::mat::NodeBuildSimplexTable;
#endif
};

}
