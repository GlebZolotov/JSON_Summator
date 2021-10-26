// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#include "../advance_tableau.hpp" // FIXME remove
#include "../node_build_simplex_table.hpp" // FIXME remove

#include "../node.hpp"

#include "test_node_constructor.hpp"

using index_vector = std::vector<uint16_t>; // FIXME

#define MAT_LOAD(name, type) \
    io::mat::load<type>(file, format("r_{}_{}{}", row, prefix, #name))

#define MAT_LOAD_T(name, mat_type, type) \
    (type)io::mat::load<mat_type>(file, format("r_{}_{}{}", row, prefix, #name))

#define MAT_LOAD_ALL(dest) \
    dest m_lowerBound = MAT_LOAD(LowerBound, double); \
    dest m_nu_lower = MAT_LOAD(LowerNuBound, d_cvector); \
    dest m_nu_upper = MAT_LOAD(UpperNuBound, d_cvector); \
    dest m_bifurcationIndex = MAT_LOAD(BifurcationIndex, uint16_t); \
    dest m_bifurcationValueLow = MAT_LOAD(BifurcationValueLow, uint16_t); \
    dest m_bifurcationValueHigh = MAT_LOAD(BifurcationValueHigh, uint16_t); \
    dest m_isOpen = MAT_LOAD_T(nodeOpen, double, bool); \
    dest m_isRoot = MAT_LOAD_T(isRootNode, double, bool); \
    dest m_isLeaf = MAT_LOAD_T(isLeaf, double, bool); \
    dest m_nuRelax = MAT_LOAD(nuRelax, d_cvector); \
    dest m_roundValue = MAT_LOAD(roundValue, double); \
    dest m_depth = MAT_LOAD(depth, uint16_t); \
    dest m_simplexTableau = MAT_LOAD(simplexTableau, d_matrix); \
    dest m_basicSet = MAT_LOAD(basicSet, u_rvector); \
    dest m_non_determined_nu = MAT_LOAD(non_determined_nu, std::vector<uint16_t>);

#define TEST(node1, node2, name) row_ok &= is_equal(node1.name, node2.name, #node1 "." #name)

#define TEST_ALL(node1, node2) \
    TEST(node1, node2, m_lowerBound); \
    TEST(node1, node2, m_nu_lower); \
    TEST(node1, node2, m_nu_upper); \
    TEST(node1, node2, m_bifurcationIndex); \
    TEST(node1, node2, m_bifurcationValueLow); \
    TEST(node1, node2, m_bifurcationValueHigh); \
    TEST(node1, node2, m_isOpen); \
    TEST(node1, node2, m_isRoot); \
    TEST(node1, node2, m_isLeaf); \
    TEST(node1, node2, m_nuRelax); \
    TEST(node1, node2, m_roundValue); \
    TEST(node1, node2, m_depth); \
    TEST(node1, node2, m_simplexTableau); \
    TEST(node1, node2, m_basicSet); \
    TEST(node1, node2, m_non_determined_nu);

inline
void
fix_index(index_vector& v)
{
    // FIX indexing, Matlab is 1-based
    for (auto& i : v)
    {
        --i;
    }
};

namespace mad_min
{

// FIXME move into separate file
BestIntegerSolution::Ptr
BestIntegerSolution::mat_load(std::string_view file_name, int row, std::string_view _prefix)
{
    auto file = io::mat::open(file_name);

    std::string prefix;
    if (not _prefix.empty())
    {
        prefix = format("{}_", _prefix);
    }

    auto sol = std::make_shared<BestIntegerSolution>();
    sol->upperBound = io::mat::load<double>(file, format("r_{}_{}bis_upperBound", row, prefix));
    sol->bestNu = io::mat::load<d_cvector>(file, format("r_{}_{}bis_bestNu", row, prefix));

    return sol;
}

Node
Node::mat_load(std::string_view file_name, int row, std::string_view _prefix)
{
    auto file = io::mat::open(file_name);
    Node node{};

    std::string prefix;
    if (not _prefix.empty())
    {
        prefix = format("{}_", _prefix);
    }

    MAT_LOAD_ALL(node.);
//     node.commonParams = Parameters::mat_load(file.name(), row, prefix + "params");
    node.m_params = Parameters::mat_load(file.name(), row, _prefix);
    node.bestSolution = BestIntegerSolution::mat_load(file.name(), row, _prefix);
    node.m_stats = std::make_shared<Stats>();

    fix_index(node.m_non_determined_nu);
    node.m_basicSet.array() -= 1;
    node.m_bifurcationIndex -= 1;

    return node;
}

bool
Node::mat_test(std::string_view file_name, int row, std::string_view prefix)
{
    using namespace tests;

    bool row_ok = true;

    auto& node = *this;
    auto hdf_node = Node::mat_load(file_name, row, prefix);
    TEST_ALL(node, hdf_node);

    return row_ok;
}

}

namespace mad_min::tests::mat
{

bool
NodeConstructorTest::test_row(MatFile& file, uint32_t row, bool verbose)
{
    bool row_ok = true;

    // TODO add tests for is_equal() functions
//     is_equal( d_inf,  d_inf, "+inf_+inf");
//     is_equal( d_inf, -d_inf, "+inf_-inf");
//     is_equal(-d_inf, -d_inf, "-inf_-inf");
//     is_equal( d_inf,  d_nan, "+inf_nan ");
//     is_equal(-d_inf,  d_nan, "-inf_nan ");
//     is_equal( d_inf,  d_nan, "+inf_nan ");
//     is_equal( d_inf,  1.0,   "+inf_1   ");
//     is_equal(-d_inf,  1.0,   "-inf_1   ");
//     is_equal( d_nan,  1.0,   " nan_1   ");
//     print("\n");
//     exit(0);

//     auto hdf_node = Node::mat_load(file.name(), row, "node1");
//     auto hdf_node = Node::mat_load(file.name(), row, "node1a");
//     auto hdf_node = Node::mat_load(file.name(), row, "node2");
//     auto hdf_node = Node::mat_load(file.name(), row, "node3");
//     auto hdf_node = Node::mat_load(file.name(), row, "node4");
    auto hdf_node = Node::mat_load(file.name(), row, "node_last");

    auto params = Parameters::mat_load(file.name(), row);
    auto bis = BestIntegerSolution::mat_load(file.name(), row);
    auto stats = std::make_shared<Stats>();

    auto nu_lower = io::mat::load<d_cvector>(file, format("r_{}_nu_lower", row));
    auto nu_upper = io::mat::load<d_cvector>(file, format("r_{}_nu_upper", row));

    auto is_root_node = (bool)io::mat::load<double>(file, format("r_{}_is_root_node", row));
    auto nodeDepth = io::mat::load<uint16_t>(file, format("r_{}_nodeDepth", row));

    if (hdf_node.m_isRoot)
    {
        auto node = Node(params, nu_lower, nu_upper, bis, stats, nullptr, verbose);
        TEST_ALL(node, hdf_node);
    }
    else
    {
        auto parent_node = Node::mat_load(file.name(), row, "parentNode");
        auto node = Node(params, nu_lower, nu_upper, bis, stats, &parent_node, verbose);
        TEST_ALL(node, hdf_node);
    }

    return row_ok;
}

}
