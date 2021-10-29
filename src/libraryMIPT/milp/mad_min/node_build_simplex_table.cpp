// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Roland Hildebrand <roland.hildebrand@univ-grenoble-alpes.fr>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#include "node_build_simplex_table.hpp"

#include "maximize_return_on_cube.hpp"
#include "utils.hpp"

namespace mad_min
{

bool
SimplexTableBuilder::build(
    const Parameters& params,
    const d_cvector& LowerNuBound,
    const d_cvector& UpperNuBound)
{
    auto r = maximize_return_on_cube(params, LowerNuBound, UpperNuBound);
    if (not(r.is_feasible) || (r.value < params.mu0))
    {
        return false;
    }

    // node.non_determined_nu = find(node.LowerNuBound ~= node.UpperNuBound);
    // n = length(node.non_determined_nu);
    // determined_nu = 1:node.commonParams.numshares;
    // determined_nu(node.non_determined_nu) = []; % indices of nu_i such that l_i = nu_i = u_i
    determined_nu.clear();
    non_determined_nu.clear();
    for (uint16_t i = 0; i < params.W; ++i)
    {
        if (LowerNuBound[i] == UpperNuBound[i])
        {
            determined_nu.push_back(i);
        }
        else
        {
            non_determined_nu.push_back(i);
        }
    }
    const auto n = non_determined_nu.size();

    Tab = params.zeroTableau;

    // Tab(1+(1:node.commonParams.numshares),end) = double(node.LowerNuBound);
    Tab.middleRows(1, params.W).rightCols(1) = LowerNuBound;

    // Tab(node.commonParams.numshares+1+(1:node.commonParams.numshares),end) = double(node.UpperNuBound);
    Tab.middleRows(1 + params.W, params.W).rightCols(1) = UpperNuBound;

    // eliminate nu_i and the corresponding slacks from the table
    // Tab(:,end) = Tab(:,end) - Tab(:,determined_nu)*nu(determined_nu);
    Tab.rightCols(1) -= Tab(Eigen::all, determined_nu) * r.nu(determined_nu);

    // Tab(:,[determined_nu, node.commonParams.numshares + determined_nu, 2*node.commonParams.numshares + determined_nu]) = [];
    for (auto i : reverse(determined_nu))
    {
        removeColumn(Tab, 2 * params.W + i);
    }
    for (auto i : reverse(determined_nu))
    {
        removeColumn(Tab, params.W + i);
    }
    for (auto i : reverse(determined_nu))
    {
        removeColumn(Tab, i);
    }

    // Tab([1 + determined_nu, 1 + node.commonParams.numshares + determined_nu],:) = [];
    for (auto i : reverse(determined_nu))
    {
        removeRow(Tab, 1 + params.W + i);
    }
    for (auto i : reverse(determined_nu))
    {
        removeRow(Tab, 1 + i);
    }

    // compute basic index set from solution
    // Mnu = node.commonParams.M*nu
    // pos_a = uint16(find(Mnu > 0)'); % positive elements of a
    // ind_b = uint16(1:node.commonParams.T);
    // ind_b(pos_a) = []; % potentially positive elements of b
    pos_a.clear();
    pos_b.clear();
    for (Eigen::Index i = 0; i < params.M.rows(); ++i)
    {
        const auto d = params.M.row(i).dot(r.nu);
        if (d > 0.0)
        {
            pos_a.push_back(i);
        }
        else
        {
            pos_b.push_back(i);
        }
    }

    // these are basic independent of the situation with the wmin and mu0 constraints
    // ind_sab = [3*(n+1), 3*(n+1)+pos_a, 3*(n+1)+node.commonParams.T+ind_b];
    ind_sab.resize(1 + pos_a.size() + pos_b.size());
    ind_sab[0] = 3 * n + 2;
    for (uint16_t i = 0; i < pos_a.size(); ++i)
    {
        ind_sab[i + 1] = 3 * (n + 1) + pos_a[i];
    }
    for (uint16_t i = 0; i < pos_b.size(); ++i)
    {
        ind_sab[i + 1 + pos_a.size()] = 3 * (n + 1) + params.T + pos_b[i];
    }

    if (r.min_max == 0) // both constraints on Gr'*nu are not active and the corresponding slacks are basic
    {
        // FIXME not implemented
        throw std::runtime_error("not implemented");

        // whether nu_i = l_i or u_i depends on the sign of rbar_i
        // rbar_pos = find(node.commonParams.rbar(node.non_determined_nu) > 0);
        // rbar_nonpos = 1:n;
        // rbar_nonpos(rbar_pos) = [];
        // Bas = [1:n, n+rbar_pos, 2*n+rbar_nonpos, 3*n+(1:2), ind_sab];
    }
    else // one of the constraints on Gr'*nu is active (depending on min_max = -1 or +1) and one of the slacks is basic
    {
        const auto kk = (3 - r.min_max) / 2;

        // [~,low_ind] = intersect(node.non_determined_nu,node.commonParams.ind_rho_div_Gr(1:index1));
        // [~,upp_ind] = intersect(node.non_determined_nu,node.commonParams.ind_rho_div_Gr(index1:node.commonParams.numshares));
        intersect(params, r.index1);

        // Bas = [1:n, n+low_ind', 2*n+upp_ind', 3*n+kk, ind_sab];
        Bas.resize(n + low_ind.size() + upp_ind.size() + 1 + ind_sab.size());
        Bas <<
            u_rvector::LinSpaced(n, 0, n - 1),
            Eigen::Map<u_rvector>(low_ind.data(), low_ind.size()).array() + n,
            Eigen::Map<u_rvector>(upp_ind.data(), upp_ind.size()).array() + 2 * n,
            3 * n + kk - 1,
            Eigen::Map<u_rvector>(ind_sab.data(), ind_sab.size());
    }

    // creating simplex table in normal form
    // Tab = [[1; zeros(2*n+3+node.commonParams.T,1)], Tab(:,Bas)]\Tab;
    // FIXME
    A = d_matrix(1 + Bas.cols(), Tab.rows());
    A.leftCols(1).array() = 0.0;
    A(0, 0) = 1.0;
    A.rightCols(Bas.cols()) = Tab(Eigen::all, Bas);
    B = Tab;

    // https://eigen.tuxfamily.org/dox/group__TutorialLinearAlgebra.html
//     Tab = A.partialPivLu().solve(B);
    Tab = A.fullPivLu().solve(B);
//     Tab = A.householderQr().solve(B);
//     Tab = A.colPivHouseholderQr().solve(B);
//     Tab = A.fullPivHouseholderQr().solve(B);
//     Tab = A.completeOrthogonalDecomposition().solve(B);
//     Tab = A.llt().solve(B); // FAIL, NaN
//     Tab = A.ldlt().solve(B); // FAIL, 10^23
//     Tab = A.bdcSvd().solve(B); // FAIL, not compiles
//     Tab = A.jacobiSvd().solve(B); // FAIL, runtime assert

    // removing rows and columns corresponding to nu
    // Bas(1:n) = []; % the variables nu are the first n basic columns
    // Bas = Bas - n;
    Bas = Bas.rightCols(Bas.cols() - n).eval(); // FIXME eval ?
    Bas.array() -= n;

    // Tab(:,1:n) = [];
    Tab = Tab.rightCols(Tab.cols() - n).eval(); // FIXME eval ?

    // Tab(1+(1:n),:) = [];
    auto bs = Tab.rows() - n - 1;
    Tab.middleRows(1, bs) = Tab.bottomRows(bs).eval();
    Tab.conservativeResize(bs + 1, Tab.cols()); // FIXME

    return true;
}

void
SimplexTableBuilder::intersect(const Parameters& params, int index1)
{
    constexpr uint16_t a_bit = 1;
    constexpr uint16_t b_bit = 2;
    constexpr uint16_t c_bit = 4;

    auto& a = non_determined_nu;
    assert(std::is_sorted(a.begin(), a.end()));

    auto& b = params.ind_rho_div_Gr.leftCols(index1 + 1);
    auto& c = params.ind_rho_div_Gr.rightCols(params.ind_rho_div_Gr.cols() - index1);

    auto& idx_ab = low_ind;
    idx_ab.clear();

    auto& idx_ac = upp_ind;
    idx_ac.clear();

    auto& s = temp;
    s.resize(params.W);

    for (auto value: a)
    {
        assert(value < params.W);
        s[value] = a_bit;
    }
    for (auto value: b)
    {
        assert(value < params.W);
        s[value] |= b_bit;
    }
    for (auto value: c)
    {
        assert(value < params.W);
        s[value] |= c_bit;
    }

    for (uint16_t i = 0; i < a.size(); ++i)
    {
        const auto value = a[i];

        if ((s[value] & a_bit) && (s[value] & b_bit))
        {
            s[value] &= ~b_bit;
            idx_ab.push_back(i);
        }

        if ((s[value] & a_bit) && (s[value] & c_bit))
        {
            s[value] &= ~c_bit;
            idx_ac.push_back(i);
        }
    }
}

}
