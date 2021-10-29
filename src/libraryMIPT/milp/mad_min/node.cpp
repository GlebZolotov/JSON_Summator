// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Roland Hildebrand <roland.hildebrand@univ-grenoble-alpes.fr>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#include "tighten_bounds.hpp"
#include "maximize_return_on_cube.hpp"
#include "quick_return_to_feasibility.hpp"
#include "return_table_to_feasibility.hpp"
#include "advance_tableau.hpp"
#include "node_build_simplex_table.hpp"
#include "integer_local_improve.hpp"
#include "utils.hpp"

#include "node.hpp"

namespace mad_min
{

Node::Node(
    const Parameters::Ptr params,
    const d_cvector& _nu_lower,
    const d_cvector& _nu_upper,
    BestIntegerSolution::Ptr _best_solution,
    Stats::Ptr stats,
    Node* parent,
    bool verbose
)
    : m_verbose(verbose)
    , m_parent(parent)
    , m_nu_lower(_nu_lower)
    , m_nu_upper(_nu_upper)
    , m_stats(stats)
    , bestSolution(_best_solution)
    , m_params(params)
    , m_isRoot(m_parent == nullptr)
    , m_depth((m_parent == nullptr) ? 0 : m_parent->m_depth + 1)
{
    stats->node_count += 1;
}

Node::~Node()
{
    m_stats->node_count -= 1;
}

void
Node::setup()
{
    // FIXME
    auto& Tab = m_simplexTableau;
    auto& Bas = m_basicSet;

    if (isChild())
    {
        tighten_bounds(*m_params, m_nu_lower, m_nu_upper);
        if ((m_nu_upper - m_nu_lower).minCoeff() < 0.0) // box for integer variables is empty
        {
//             LowerBound = d_inf;
//             nodeOpen = false;
//             roundValue = d_inf;

            close();

            return;
        }

        // elseif sum(abs(double(nu_upper) - double(nu_lower))) == 0
        else if ((m_nu_upper - m_nu_lower).lpNorm<1>() == 0.0) // only one integer point is potentially feasible
        {
            // nu = double(nu_lower);
            // wGr = commonParameters.Gr'*nu;
            auto wGr = m_params->Gr.dot(m_nu_lower);

            // if (commonParameters.mu0 <= commonParameters.rbar*nu) &&
            //    (commonParameters.wmin <= wGr) &&
            //    (wGr <= 1) % point feasible
            if ((m_params->mu0 <= m_params->rbar.dot(m_nu_lower)) &&
                (m_params->wmin <= wGr) &&
                (wGr <= 1)) // point feasible
            {
                // FIXME
                throw std::runtime_error("Not implemented");

                // value = computeMADvariance(commonParameters,nu);
                // nodeOpen = false;
                // roundValue = value;
                // LowerBound = value;
                // best_solution.setValues(value, nu);
            }
        }

        // this will be updated later by the buildSimplexTable routine
        // or when updating the parent node table
        m_non_determined_nu = m_parent->m_non_determined_nu;
    }
    else
    {
        // obj.non_determined_nu = 1:commonParameters.numshares;
        m_non_determined_nu.resize(m_params->W);
        std::iota(m_non_determined_nu.begin(), m_non_determined_nu.end(), 0);
    }

    auto n = m_non_determined_nu.size();

//     // dump node1
//     return;

    // build simplex table for the LP relaxation
    // either use the table from the parent node or construct one
    // from scratch using the solution of the feasibility check
    if (isChild()) // if there is a parent node it must have its own table
    {
        Tab = m_parent->m_simplexTableau;
        Bas = m_parent->m_basicSet;

        // update the slack values according to changed upper and
        // lower bounds
        // differences with bounds of parent node

        // delta_upper = double(nu_upper(obj.non_determined_nu)) -
        //               double(obj.ParentNode.UpperNuBound(obj.non_determined_nu)); % <= 0
        auto delta_upper = m_nu_upper(m_non_determined_nu) - m_parent->m_nu_upper(m_non_determined_nu);

        // delta_lower = double(obj.ParentNode.LowerNuBound(obj.non_determined_nu)) -
        //               double(nu_lower(obj.non_determined_nu)); % <= 0
        auto delta_lower = m_parent->m_nu_lower(m_non_determined_nu) - m_nu_lower(m_non_determined_nu);

        // delta_rhs = [delta_lower; delta_upper; zeros(3+2*commonParameters.T,1)];
        d_cvector delta_rhs(2 * m_non_determined_nu.size() + 3 + 2 * m_params->T);
        delta_rhs <<
            delta_lower,
            delta_upper,
            d_cvector::Zero(3 + 2 * m_params->T, 1);

        // if sum(sign(delta_rhs)) == -1
        if (delta_rhs.cwiseSign().sum() == -1.0) // only one equality constraint has been tightened
        {
            // j = find(delta_rhs < 0, 1);
            auto j = find1(delta_rhs, [](double x){ return x < 0.0; });
            assert(j >= 0);

            // q = find(Bas == j, 1);
            auto q = find1(Bas, equal_to(j));
            assert(q >= 0);

            // [s,Tab,Bas] = quick_return_to_feasibility(delta_rhs(j),j,q,Tab,Bas);
            bool s = quick_return_to_feasibility(delta_rhs[j], j, q, Tab, Bas, m_verbose);
            if (s == false) // no feasible point
            {
//                 LowerBound = d_inf;
//                 nodeOpen = false;
//                 roundValue = d_inf;

                close();

                return;
            }
        }
        else // several equality constraints have been tightened
        {
            // [s,Tab,Bas] = return_table_to_feasibility(delta_rhs,Tab,Bas);
            bool s = return_table_to_feasibility(delta_rhs, Tab, Bas, m_verbose);
            if (s == false) // no feasible point
            {
//                 LowerBound = d_inf;
//                 nodeOpen = false;
//                 roundValue = d_inf;

                close();

                return;
            }
        }

//         // dump node1a
//         return;

        // we now can eliminate further rows and columns corresponding to equal
        // upper and lower bounds
        //
        // ind_equal = flipud(find(nu_upper(obj.non_determined_nu) == nu_lower(obj.non_determined_nu)));
        // % sort in descending order
        std::vector<Eigen::Index> ind_equal;
        for (uint16_t i = 0; i < m_non_determined_nu.size(); ++i)
        {
            const auto nd_i = m_non_determined_nu[i];
            if (m_nu_upper[nd_i] == m_nu_lower[nd_i])
            {
                ind_equal.push_back(i);
            }
        }
        assert(std::is_sorted(ind_equal.begin(), ind_equal.end()));

        if (ind_equal.empty() == false)
        {
            for (auto k : reverse(ind_equal))
            {
                // q = find(Bas == n + k, 1);
                auto q = find1(Bas, equal_to(n + k));
                if (q >= 0)
                {
                    // Tab(q+1,:) = [];
                    removeRow(Tab, q + 1);

                    // Bas(q) = [];
                    removeColumn(Bas, q);
                }

                // Tab(:,n + k) = [];
                removeColumn(Tab, n + k);

                // Bas(Bas > n + k) = Bas(Bas > n + k) - 1;
                for (auto& b: Bas)
                {
                    if (b > (n + k))
                    {
                        --b;
                    }
                }

                // q = find(Bas == k, 1);
                q = find1(Bas, equal_to(k));
                if (q >= 0)
                {
                    // Tab(q+1,:) = [];
                    removeRow(Tab, q + 1);

                    // Bas(q) = [];
                    removeColumn(Bas, q);
                }

                // Tab(:,k) = [];
                removeColumn(Tab, k);

                // Bas(Bas > k) = Bas(Bas > k) - 1;
                for (auto& b: Bas)
                {
                    if (b > k)
                    {
                        --b;
                    }
                }

                // n = n - 1;
                --n;
            }

            // non_determined_nu(ind_equal) = [];
            for (auto i : reverse(ind_equal))
            {
                m_non_determined_nu.erase(m_non_determined_nu.begin() + i);
            }
        }
    }
    else
    {
        SimplexTableBuilder builder;
        bool is_feasible = builder.build(*m_params, m_nu_lower, m_nu_upper);
        if (not is_feasible)
        {
            close();
            return;
        }

        Tab.swap(builder.Tab);
        Bas.swap(builder.Bas);
        m_non_determined_nu.swap(builder.non_determined_nu);
        n = m_non_determined_nu.size();
    }

//     // dump node2
//     return;

    // solve LP relaxation
    int iter = 0;
    Timer tm;
    while (true)
    {
        ++iter;
        auto [opt, ub] = advance_tableau(Tab, Bas, false);
        if (opt)
        {
            break;
        }

        if (ub)
        {
            // FIXME
            print("\n\n UB !!!! \n\n");
            exit(0);
        }
    }
    m_stats->simplex_iterations += iter;

    auto value = -Tab(0, Tab.cols() - 1);
    m_lowerBound = value;

    if (m_verbose)
    {
        double t = tm.ms();
        print("Solution phase: {:4} simplex iterations in {:8.2f} ms.; {:8.4f} ms. per iteration\n",
                iter, t, t / iter);
    }

//     // dump node3
//     return;

    // checks if node has higher value than the current best integer one
    if (bestSolution->upperBound <= value) // node gives worse values than the best integer solution found so far
    {
        m_isOpen = false;
        m_roundValue = d_inf;

        m_simplexTableau.resize(0, 0);
        m_basicSet.resize(0);

        // FIXME close() ?

        return;
    }

    // Tab(2:end,end) = max(0,Tab(2:end,end)); % correct small negative values of the basic variables
    for (auto i = 1; i < Tab.rows(); ++i)
    {
        Tab(i, cend(Tab)) = std::max(0.0, Tab(i, cend(Tab)));
    }

//     // dump node4
//     return;

    // restore solution nu from slacks in the table

    // nu = double(nu_lower);
    // ...
    // obj.nuRelax = nu;
    auto& nu = m_nuRelax;
    nu = m_nu_lower;

    // ind_frac = zeros(1,n);
    u_rvector ind_frac = u_rvector::Zero(n); // index set of elements in the interior of (l_i,u_i)

    auto count_frac = 0;
    for (size_t i = 0; i < n; ++i)
    {
        // q = find(Bas == i, 1);
        auto q = find1(Bas, equal_to(i));

        // if ~isempty(q)
        if (q >= 0) // nu_i is not at lower bound
        {
            // q1 = find(Bas == n+i, 1);
            auto q1 = find1(Bas, equal_to(n + i));

            auto nd_i = m_non_determined_nu[i];

            // if isempty(q1)
            if (q1 < 0) // nu_i is at upper bound
            {
                // nu(obj.non_determined_nu(i)) = double(nu_upper(obj.non_determined_nu(i)));
                nu[nd_i] = m_nu_upper[nd_i];
            }
            else // nu_i is neither at lower nor at upper bound
            {
                // nu(obj.non_determined_nu(i)) = nu(obj.non_determined_nu(i)) + Tab(q+1,end);
                nu[nd_i] = nu[nd_i] + Tab(q + 1, cend(Tab));

                // if abs(Tab(q+1,end) - round(Tab(q+1,end))) < 10^(-9)
                if (fabs(Tab(q + 1, cend(Tab)) - round(Tab(q + 1, cend(Tab)))) < 1e-9) // by chance the intermediate value is also integer
                {
                    // nu(obj.non_determined_nu(i)) = round(nu(obj.non_determined_nu(i)));
                    nu[nd_i] = round(nu[nd_i]);
                }
                else
                {
                    // count_frac = count_frac + 1;
                    // ind_frac(count_frac) = obj.non_determined_nu(i);
                    ind_frac[count_frac++] = nd_i;
                }
            }
        }
    }

    // ind_frac = ind_frac(1:count_frac);
    ind_frac.conservativeResize(count_frac);

    // checks if node yields an integer solution
    // look for non-integer value in the solution with the highest
    // sensitivity of the cost function

    // if isempty(ind_frac)
    if (ind_frac.size() == 0)
    {
        // obtained integer solution, it is better than the best solution

        m_isOpen = false;
        m_roundValue = value;
        bestSolution->setValues(value, nu);

        m_simplexTableau.resize(0, 0);
        m_basicSet.resize(0);
    }
    else
    {
        // obtained non-integer solution is better than current integer value
        // branching index is determined by highest sensitivity of the cost function

        m_isOpen = true;
        auto x = m_params->M * nu;

        //sMx = sum(diag(sign(x))*commonParameters.M,1);
        auto sMx = (x.cwiseSign().asDiagonal() * m_params->M).colwise().sum();

        // [~,maxind] = max(abs(sMx(ind_frac)));
        Eigen::Index maxind2;
        sMx(ind_frac).cwiseAbs().maxCoeff(&maxind2);

        auto maxind = max_idx(sMx(ind_frac).cwiseAbs());
        assert(maxind == maxind2);

        // TODO BifurcationValue* -> double ?
        m_bifurcationIndex = ind_frac[maxind];
        m_bifurcationValueLow = (uint16_t)floor(nu[m_bifurcationIndex]);
        m_bifurcationValueHigh = (uint16_t)ceil(nu[m_bifurcationIndex]);

        // trying to obtain a suboptimal integer solution
        // first try with round(nu)
        // [s,value,nu_int] = integer_local_improve(commonParameters,round(nu));
//         auto nu_round = nu.array().round().eval();
        auto [s, value, nu_int] = integer_local_improve(*m_params, nu.array().round());
        m_roundValue = (s) ? value : d_inf;

        if (s && (value < bestSolution->upperBound))
        {
            bestSolution->setValues(value, nu_int);
        }

        // solving auxiliary LP on the unit cube surrounding the
        // fractional solution and trying to improve locally
        // [s,~,nu_low,nu_upp] = maximize_return_on_cube(commonParameters,uint16(floor(nu)),uint16(ceil(nu)));
        auto r = maximize_return_on_cube(*m_params, nu.array().floor(), nu.array().ceil());
        if (r.is_feasible)
        {
            {
                auto [s, value, nu_int] = integer_local_improve(*m_params, r.nu_low);
                if (s && (value < m_roundValue))
                {
                    m_roundValue = value;
                }
                if (s && (value < bestSolution->upperBound))
                {
                    bestSolution->setValues(value, nu_int);
                }
            }

            {
                auto [s, value, nu_int] = integer_local_improve(*m_params, r.nu_upp);
                if (s && (value < m_roundValue))
                {
                    m_roundValue = value;
                }
                if (s && (value < bestSolution->upperBound))
                {
                    bestSolution->setValues(value, nu_int);
                }
            }
        }
    }
}

void
Node::close()
{
    m_isOpen = false;
    m_lowerBound = d_inf;
    m_roundValue = d_inf;

    m_simplexTableau.resize(0, 0);
    m_basicSet.resize(0);
}

int
Node::bifurcate()
{
    assert(m_upperChild == nullptr);
    assert(m_lowerChild == nullptr);

    // num_integer_values = length(find(node.nuRelax == round(node.nuRelax)));
    int num_integer_values = 0;
    for (auto r: m_nuRelax)
    {
        if (r == round(r))
        {
            ++num_integer_values;
        }
    }

    // higher branch
    auto nu_lower_high = m_nu_lower;
    nu_lower_high[m_bifurcationIndex] = m_bifurcationValueHigh;
    m_upperChild = std::make_unique<Node>(
        m_params, nu_lower_high, m_nu_upper, bestSolution, m_stats, this, m_verbose);
    m_upperChild->setup();

    // lower branch
    auto nu_upper_low = m_nu_upper;
    nu_upper_low[m_bifurcationIndex] = m_bifurcationValueLow;
    m_lowerChild = std::make_unique<Node>(
        m_params, m_nu_lower, nu_upper_low, bestSolution, m_stats, this, m_verbose);
    m_lowerChild->setup();

    // FIXME fix NodeBifurcateTest() + matlab code
    m_simplexTableau.resize(0, 0);
    m_basicSet.resize(0);

    m_isLeaf = false;

    // FIXME fix NodeBifurcateTest() + matlab code
//     updateNodeValue();

    return num_integer_values;
}

bool
Node::updateNodeValue()
{
    auto old_lower_bound = m_lowerBound;
    auto old_status = m_isOpen;

    if (m_isLeaf == false) // should be true automatically
    {
        assert(m_lowerChild != nullptr);
        assert(m_upperChild != nullptr);

        m_lowerBound = std::min(m_lowerChild->m_lowerBound, m_upperChild->m_lowerBound);

        // if both child nodes are closed, we may delete them and close the parent node
        // the updated node calls the update routine for the parent node
        if (m_lowerChild->isClose() && m_upperChild->isClose())
        {
            m_lowerChild.reset();
            m_upperChild.reset();

            m_isOpen = false;
            m_isLeaf = true;
        }
    }

    if (m_isRoot)
    {
        return false;
    }

    // ask parent node for update only if something has changed
    if ((m_lowerBound != old_lower_bound) || (m_isOpen != old_status))
    {
        // during testing we have no parent
        // TODO add some field/global variable to detect testing cases
        // or extract code from this method into other method which will be tested instead of this one
//         if (ParentNode != nullptr)
        {
//             ParentNode->updateNodeValue();
            return true;
        }
    }

    return false;
}

void
Node::updateByUpperBound()
{
    if (m_lowerBound >= bestSolution->upperBound)
    {
        m_lowerChild.reset();
        m_upperChild.reset();

        m_isOpen = false;
        m_isLeaf = true;
    }
    else
    {
        if (m_isLeaf == false)
        {
            m_lowerChild->updateByUpperBound();
            m_upperChild->updateByUpperBound();
        }
    }
}

Node*
Node::findMin()
{
    if (m_isLeaf)
    {
        return this;
    }

    if (m_lowerChild->m_lowerBound < m_upperChild->m_lowerBound)
    {
        return m_lowerChild->findMin();
    }
    else
    {
        return m_upperChild->findMin();
    }
}

Node*
Node::parent() const
{
    return m_parent;
}

bool
Node::isOpen() const
{
    return m_isOpen;
}

bool
Node::isRoot() const
{
    return m_isRoot;
}

bool
Node::isLeaf() const
{
    return m_isLeaf;
}

uint16_t
Node::depth() const
{
    return m_depth;
}

double
Node::roundValue() const
{
    return m_roundValue;
}

double
Node::lowerBound() const
{
    return m_lowerBound;
}

uint16_t
Node::bifurcationIndex() const
{
    return m_bifurcationIndex;
}

}
