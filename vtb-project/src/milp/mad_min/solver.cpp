// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Roland Hildebrand <roland.hildebrand@univ-grenoble-alpes.fr>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#include "parameters.hpp"
#include "best_integer_solution.hpp"
#include "node.hpp"

#include "solver.hpp"

MADMin_Solver::MADMin_Solver(MadStatement* st)
    : m_statement(st)
{
}

MadSolution&
MADMin_Solver::Solve()
{
    using namespace mad_min;

    Timer timer;

    m_solution.status = SolutionStatus::NOT_STARTED;
    m_solution.problemType = ProblemType::MAD_MIN;

    const int iter_max = m_statement->iterationLimit;
    const int nodes_max = m_statement->nodeLimit;
    const double time_max = m_statement->timeLimit;
    print("{:e}\n", time_max);

    const uint16_t W = m_statement->timeRate.rows();
    const uint16_t T = m_statement->timeRate.cols();
    const double p_max = m_statement->maxWeight(0); // FIXME
    const double mu0 = m_statement->minReturn;

    const double beta = 0.95; // FIXME
    const double bTcoef = 1.0 / ((1.0 - beta) * T);
    const double w_min = m_statement->minSize;

    cvector<uint16_t> numax(W);
    d_cvector Gr(W);
    for(int i = 0; i < m_statement->close.size(); ++i)
    {
        Gr[i] = m_statement->close[i] * m_statement->lotSize[i] / m_statement->capital;
        numax[i] = (uint16_t)floor(p_max / Gr[i]);
    }

    d_matrix retG(T, W);
    retG = m_statement->timeRate.transpose();
    for (uint16_t i = 0; i < T; ++i)
    {
        for (uint16_t j = 0; j < W; ++j)
        {
            retG(i, j) *= Gr[j];
        }
    }

    auto params = std::make_shared<Parameters>(W, T, bTcoef, mu0, w_min, retG, Gr, numax);
//     params->h5_save("solver_params.h5");

    auto nu_lower = d_cvector::Zero(params->W).eval();
    auto nu_upper = params->numax.cast<double>().eval();

    auto bis = std::make_shared<BestIntegerSolution>();
    bis->setValues(d_inf, nu_upper);
    auto old_best_value = bis->upperBound;

    auto stats = std::make_shared<Stats>();

    bool verbose = false;
    auto root_node = Node(params, nu_lower, nu_upper, bis, stats, nullptr, verbose);
    root_node.setup();

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

    int iter = 0;
    double time_i = 0.0;
    double gap = d_inf;

    while (true)
    {
        if (root_node.isOpen() == false)
        {
            m_solution.status = (bis->bestNu.size() > 0)
                ? SolutionStatus::OPTIMAL_FOUND
                : SolutionStatus::INCORRECT_PROBLEM;
            break;
        }

        if (iter >= iter_max)
        {
            m_solution.status = SolutionStatus::ITERATION_LIMIT_REACHED;
            break;
        }

        if (time_i >= time_max)
        {
            m_solution.status = SolutionStatus::TIME_LIMIT_REACHED;
            break;
        }

        if (stats->node_count >= nodes_max)
        {
            m_solution.status = SolutionStatus::NODE_LIMIT_REACHED;
            break;
        }

        if (std::isfinite(bis->upperBound) && (gap <= 0.0001))
        {
            m_solution.status = SolutionStatus::SUBOPTIMAL_FOUND;
            break;
        }

        // branch at node with minimal LP relaxation value
        auto min_node = root_node.findMin();

        const auto round_value = min_node->roundValue(); // when this is printed the node might no more exist
        const auto bifurcation_index = min_node->bifurcationIndex();
        const auto depth = min_node->depth();
        const auto oldval = root_node.lowerBound();
        const auto num_integer_values = min_node->bifurcate();

        auto node = min_node;
        while (node->updateNodeValue())
        {
            node = node->parent();
        }

        if (oldval > root_node.lowerBound())
        {
            // this is be able to analyze the problem if this case occurs (probably this was
            // due to the numerical errors in the cvx solution)
            // FIXME
            print("\n\n ERROR \n\n");
            exit(-1);
        }

        if (old_best_value > bis->upperBound)
        {
            old_best_value = bis->upperBound;
            root_node.updateByUpperBound();
        }

        ++iter;
        time_i = timer.s();
        print("\r{:7.2f} | {:5d} | {:12.6e} | {:12.6e} | {:12.6e} | {:4} | {:4} | {:5} | {:5}",
              time_i,
              iter,
              root_node.lowerBound(),
              bis->upperBound,
              round_value,
              num_integer_values,
              bifurcation_index,
              depth,
              stats->node_count
              );

        if (std::isfinite(bis->upperBound))
        {
            gap = bis->upperBound / root_node.lowerBound() - 1.0;
            print(" | {:5.2f}", gap * 100.0);
        }

        if (iter == 1)
        {
            print("\n");
        }

        m_solution.nodeAmount = stats->node_count;
        m_solution.iterationAmount = iter;
        m_solution.time = time_i;
        m_solution.accuracy = gap;
    }
    print("\n");

    switch (m_solution.status)
    {
        case SolutionStatus::INCORRECT_PROBLEM:
            print("No feasible integer point found\n");
            break;

        case SolutionStatus::OPTIMAL_FOUND:
            print("Optimal value: {:5e}\n", bis->upperBound);
            break;

        case SolutionStatus::SUBOPTIMAL_FOUND:
            print("Relative gap value achieved: {:.5g} <= 0.0001\n", gap);
            break;

        case SolutionStatus::ITERATION_LIMIT_REACHED:
            print("Iteration limit reached: {} >= {}\n", iter, iter_max);
            break;

        case SolutionStatus::TIME_LIMIT_REACHED:
            print("Time limit reached: {} >= {}\n", time_i, time_max);
            break;

        case SolutionStatus::NODE_LIMIT_REACHED:
            print("Node limit reached: {} >= {}\n", stats->node_count, nodes_max);
            break;

        case SolutionStatus::NOT_STARTED:
        case SolutionStatus::STARTED:
        case SolutionStatus::UNBOUNDED_PROBLEM:
        case SolutionStatus::INTERRUPTED:
        case SolutionStatus::NUMERICAL_ERROR:
        case SolutionStatus::USER_OBJ_LIMIT_REACHED:
        case SolutionStatus::NOT_SUPPORTED:
            break;
    };

    m_solution.objectiveValue = bis->upperBound;
    m_solution.numLots = m_statement->lotSize;
    m_solution.weights = m_statement->maxWeight;
    
    return Current();
}

void
MADMin_Solver::Interrupt()
{
}

MadSolution&
MADMin_Solver::Current()
{
    return m_solution;
}
