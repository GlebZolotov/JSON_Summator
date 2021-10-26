// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Roland Hildebrand <roland.hildebrand@univ-grenoble-alpes.fr>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#include "node.hpp"

int
main(int /*argc*/, char ** /*argv*/)
{
    using namespace mad_min;

    Timer t0;

//     auto fname = "../matlab/MADmin/test.mat";
//     auto fname = "../matlab/MADmin/test_036_107.mat";
//     auto fname = "../matlab/MADmin/test_052_102.mat";
    auto fname = "../matlab/MADmin/test_056_114.mat";
//     auto fname = "../matlab/MADmin/test_059_120.mat";
//     auto fname = "../matlab/MADmin/test_060_107.mat";
//     auto fname = "../matlab/MADmin/test_064_107.mat";
//     auto fname = "../matlab/MADmin/test_066_119.mat";
//     auto fname = "../matlab/MADmin/test_074_108.mat";
//     auto fname = "../matlab/MADmin/test_086_119.mat";
//     auto fname = "../matlab/MADmin/test_096_111.mat";

    auto params = Parameters::mat_load(fname, 1);

    auto nu_lower = d_cvector::Zero(params->W).eval();
    auto nu_upper = params->numax.cast<double>().eval();

    auto bis = std::make_shared<BestIntegerSolution>();
    bis->setValues(d_inf, nu_upper);
    auto old_best_value = bis->upperBound;

    auto stats = std::make_shared<Stats>();

    bool verbose = false;
    auto rootNode = Node(params, nu_lower, nu_upper, bis, stats, nullptr, verbose);
    rootNode.setup();

//     // FIXME tests
//     {
//         auto mat_node = Node::mat_load(fname, 1, "root");
//         rootNode.m_simplexTableau = mat_node.m_simplexTableau;
//         print("!!! MATLAB table !!!\n");
//     }

    print("Time horizon: {}, Number of shares: {}, w_min = {:e}\n",
          params->T,
          params->W,
          params->wmin);

    print("{:>7} | {:>5} | {:12} | {:12} | {:12} | {:4} | {:4} | {:5} | {:5} | {:>5}\n",
          "time",
          "iter",
          "low_bound",
          "up_bound",
          "loc_int_sol", // Local Integer Solution
          "#int", // # integer values
          "br_i", // branch index
          "depth",
          "nodes",
          "gap"
    );

    auto Nmax = int(1e6);
    auto countIter = 0;
    auto relGap = d_inf;

    // while rootNode.nodeOpen && (countIter < Nmax) && (isinf(best_solution.upperBound) || (relGap > 0.0001))
    while (
        rootNode.isOpen() &&
        (countIter < Nmax) &&
        ((std::isfinite(bis->upperBound) == false) || (relGap > 0.0001)))
    {
        // branch at node with minimal LP relaxation value
        auto minNode = rootNode.findMin();

        auto rV = minNode->roundValue(); // when this is printed the node might no more exist
        auto bfI = minNode->bifurcationIndex();
        auto nDpth = minNode->depth();
        auto oldval = rootNode.lowerBound();
        auto num_integer_values = minNode->bifurcate();

        auto node = minNode;
        while (node->updateNodeValue())
        {
            node = node->parent();
        }

        if (oldval > rootNode.lowerBound())
        {
            // this is be able to analyze the problem if this case occurs (probably this was
            // due to the numerical errors in the cvx solution)
            print("\n\n ERROR \n\n");
            exit(-1);
        }

        if (old_best_value > bis->upperBound)
        {
            old_best_value = bis->upperBound;
            rootNode.updateByUpperBound();
        }

        ++countIter;
        print("\r{:7.2f} | {:5d} | {:12.6e} | {:12.6e} | {:12.6e} | {:4} | {:4} | {:5} | {:5}",
              t0.s(),
              countIter,
              rootNode.lowerBound(),
              bis->upperBound,
              rV,
              num_integer_values,
              bfI,
              nDpth,
              stats->node_count
              );

        if (std::isfinite(bis->upperBound))
        {
            relGap = bis->upperBound / rootNode.lowerBound() - 1.0;
            print(" | {:5.2f}", relGap * 100.0);
        }

        if (countIter == 1)
        {
            print("\n");
        }
    }
    print("\n");

    if (rootNode.isOpen())
    {
//         if (relGap < 0.0001) && (~isempty(best_solution.bestNu))
        if (relGap < 0.0001)
        {
            print("Relative gap of 0.0001 achieved in {} iterations\n", countIter);
        }
        else
        {
            print("Maximal number of iterations achieved\n");
        }
    }
    else
    {
        print("Optimal value: {:5e}\n", bis->upperBound);
    }

//     if isempty(best_solution.bestNu)
//         fprintf("No feasible integer point found")
//     end

    return 0;
}
