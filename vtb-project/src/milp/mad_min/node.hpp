// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Roland Hildebrand <roland.hildebrand@univ-grenoble-alpes.fr>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#pragma once

#include "best_integer_solution.hpp"
#include "parameters.hpp"

namespace mad_min
{

#if defined HDF5_TESTS
namespace tests::mat
{

class NodeConstructorTest;
class NodeBifurcateTest;

}
#endif

struct Stats
{
    using Ptr = std::shared_ptr<Stats>;

    int node_count = 0;
    int simplex_iterations = 0;
};

class Node
{
public:
    // creates a new node
    // if the argument parentNode is empty, this is the root node
    // when it is created, the node solves its LP relaxation and
    // memorizes the corresponding simplex table
    // if the value is above the value of the current best solution, the
    // node declares itself closed
    // if the LP solution is integer, then the node is also closed
    // if the integer solution is better than the current best, it
    // replaces the best solution
    // if the value is lower than the current best and not integer, the
    // node declares itself as open and computes the index of the
    // variable for branching
    Node(
        const Parameters::Ptr params,

        // W x 1
        const d_cvector& nu_lower,

        // W x 1
        const d_cvector& nu_upper,

        BestIntegerSolution::Ptr best_solution,
        Stats::Ptr stats,
        Node* parent = nullptr,
        bool verbose = false
    );

    ~Node();

    void
    setup();

    // bifurcates by creating child nodes
    // uses the previously computes index and values of the branching
    // variable
    // gives back the number of integer values in the vector of integer
    // variables
    int
    bifurcate();

    // if values (LowerBound) of child nodes change, they call this update function of
    // their parent node
    // it is also called by the bifurcate method when the child nodes are
    // created
    // updates the lower bound and the open/closed status of the node
    // since the method is called, it is assumed that the child nodes
    // exist
    bool
    updateNodeValue();

    // is called if the best integer solution has improved
    // this affects all nodes, because their values can become worse than
    // the new upper bound
    // in this case the node is closed and its child nodes can be deleted
    // function is called only for the root node, the call then
    // propagates downwards the whole tree
    void
    updateByUpperBound();

    // returns the leaf node with the minimal value attached to the input
    // node
    // function operates recursively
    // uses the fact that the value of the parent node is always the
    // minimum of the child node values
    // hence we just descend the minimal branch
    // the minimal of the two child nodes is active, otherwise the child
    // nodes would have been removed already
    Node*
    findMin();

    Node*
    parent() const;

    bool
    isOpen() const;

    bool
    isClose() const
    {
        return !isOpen();
    }

    bool
    isRoot() const;

    bool
    isChild() const
    {
        return !isRoot();
    }

    uint16_t
    depth() const;

    bool
    isLeaf() const;

    double
    lowerBound() const;

    double
    roundValue() const;

    uint16_t
    bifurcationIndex() const;

private:
    void
    close(); // TODO rename ?

    bool m_verbose = false;

    double m_lowerBound = 0.0;
    Node* m_parent = nullptr;
    std::unique_ptr<Node> m_lowerChild;
    std::unique_ptr<Node> m_upperChild;

    /// lower bounds on integer variables
    d_cvector m_nu_lower;

    /// upper bounds on integer variables
    d_cvector m_nu_upper;

    /// index of integer variable according to which child nodes are created
    uint16_t m_bifurcationIndex = 0;

    /// lower integer value of branching variable
    uint16_t m_bifurcationValueLow = 0;

    /// higher integer value of branching variable
    uint16_t m_bifurcationValueHigh = 0;

    /// determines whether the node has to be further investigated
    bool m_isOpen;

    Stats::Ptr m_stats;
    BestIntegerSolution::Ptr bestSolution;
    Parameters::Ptr m_params;

    /// if true then this is the root node
    bool m_isRoot;

    /// if true then there are no child nodes
    bool m_isLeaf = true;

    /// vector of integer variables obtained from LP relaxation, entries may be non-integer
    d_cvector m_nuRelax;

    /// value of integer point obtained by rounding procedure
    double m_roundValue = 0.0;

    /// depth of node in the tree, the root node has depth 0
    uint16_t m_depth;

    /// simplex tableau of the optimal solution of the LP relaxation
    d_matrix m_simplexTableau;

    /// basic set of simplex tableau
    u_rvector m_basicSet;

    // the variables corresponding to the columns are:
    // slack (l <= nu), slack(nu <= u), slack(wmin <= Gr*nu),
    // slack(Gr*nu <= 1), slack(rbar*nu >= mu0), a, b

    // index set of nu_i which are not fixed by lower and upper bounds
    // 1 x W for root
    // 1 x Q for childs (Q <= W)
    std::vector<uint16_t> m_non_determined_nu;

#if defined HDF5_TESTS
    friend class tests::mat::NodeConstructorTest;
    friend class tests::mat::NodeBifurcateTest;

public:
    static
    Node
    mat_load(std::string_view file_name, int row, std::string_view prefix = {});

    bool
    mat_test(std::string_view file_name, int row, std::string_view prefix = {});

private:
    Node() = default;
    Node(Node&&) = default;
#endif
};

}
