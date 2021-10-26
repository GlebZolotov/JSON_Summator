// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#include "../advance_tableau.hpp" // FIXME remove
#include "../node_build_simplex_table.hpp" // FIXME remove

#include "../node.hpp"

#include "test_node_bifurcate.hpp"

using index_vector = std::vector<uint16_t>;

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

namespace mad_min::tests::mat
{

bool
NodeBifurcateTest::test_row(MatFile& file, uint32_t row, bool verbose)
{
    bool row_ok = true;

    auto node = Node::mat_load(file.name(), row, "node_start");
    node.m_verbose = verbose;

//     Node node_parent;
//     Node node_parent_hdf;
//     if (node.isRootNode == false)
//     {
//         node_parent = Node::mat_load(file.name(), row, "node_ParentNode");
//         node_parent_hdf = Node::mat_load(file.name(), row, "node_ParentNode");
//         node.ParentNode = &node_parent;
//     }

    auto num_integer_values = node.bifurcate();
    node.updateNodeValue();

    {
        auto num_integer_values_hdf = (int)io::mat::load<double>(file, format("r_{}_num_integer_values", row));
        row_ok &= is_equal(num_integer_values, num_integer_values_hdf, "num_integer_values");
    }

    {
        auto node_hdf = Node::mat_load(file.name(), row, "node_end");
        TEST_ALL(node, node_hdf);
    }

    if (node.m_upperChild != nullptr)
    {
        auto node_upper_hdf = Node::mat_load(file.name(), row, "node_UpperChildNode");
        auto& node_upper = *(node.m_upperChild);
        TEST_ALL(node_upper, node_upper_hdf);
    }

    if (node.m_lowerChild != nullptr)
    {
        auto node_lower_hdf = Node::mat_load(file.name(), row, "node_LowerChildNode");
        auto& node_lower = *(node.m_lowerChild);
        TEST_ALL(node_lower, node_lower_hdf);
    }

//     if (node.isRootNode == false)
//     {
//         TEST_ALL(node_parent, node_parent_hdf);
//     }

    return row_ok;
}

}
