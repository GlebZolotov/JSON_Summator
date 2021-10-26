// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Roland Hildebrand <roland.hildebrand@univ-grenoble-alpes.fr>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#include "advance_tableau.hpp"
#include "utils.hpp"

#include "return_table_to_feasibility.hpp"

namespace mad_min
{

bool
return_table_to_feasibility(
    const d_cvector& delta_slack,
    d_matrix& Tab,
    u_rvector& Bas,
    bool verbose)
{
    // n = length(delta_slack);
    auto n = delta_slack.size();
    assert(Tab.cols() == (n + 1));

    d_matrix Tab1(Tab.rows() + 2, n + 3);
    Tab1 <<
        // zeros(1,n), -1, 0, -1;
        d_matrix::Zero(1, n),
        -1.0,
        0.0,
        -1.0,

        // Tab(:,1:n), -Tab(:,1:n) * delta_slack, zeros(size(Tab,1),1), Tab(:,end);
        Tab.leftCols(n),
        Tab.leftCols(n) * (-delta_slack),
        d_matrix::Zero(Tab.rows(), 1),
        Tab.rightCols(1),

        // zeros(1,n), ones(1,3)];
        d_matrix::Zero(1, n),
        d_matrix::Ones(1, 3);

    // Bas = [Bas, n+2];
    u_rvector Bas1(Bas.size() + 1);
    Bas1 << Bas, n + 1;

    int count_advance = 0;
    Timer tm;
    while (true)
    {
        ++count_advance;
        auto [opt, ub] = advance_tableau(Tab1, Bas1, true);
        if (opt)
        {
            break;
        }
    }

    if (verbose)
    {
        double t = tm.ms();
        print("TR Phase 1 from parent table: {:4} simplex iterations in {:8.2f} ms.; {:8.4f} ms. per iteration\n",
                count_advance, t, t / count_advance);
    }

    // qz = find(Bas == n+2, 1);
    const auto qz = find1(Bas1, equal_to(n + 1));

    // if ~isempty(qz) && (Tab(1,end) > -10^(-12))
    if ((qz >= 0) && (Tab1(0, cend(Tab1)) > -1e-12)) // z variable is still basic, but reached zero
    {
        // find a non-basic variable which can be made basic and replace z
        // the corresponding element in the row qz has to be positive

        // non_bas = 1:n+2;
        // non_bas(Bas) = [];
        // FIXME ugly and diry code
        std::vector<uint16_t> non_bas(n + 1);
        std::iota(non_bas.begin(), non_bas.end(), 0);
        for (auto i = 0; i < Bas1.size(); ++i)
        {
            non_bas[Bas1[i]] = -1;
        }
        non_bas.erase(std::remove(non_bas.begin(), non_bas.end(), (uint16_t)-1), non_bas.end());

        // [~,jind] = min(Tab(qz+2,non_bas)); % max -> min, fixed by Roland
        uint16_t jind;
        Tab1(qz + 2, non_bas).minCoeff(&jind);

        // j = non_bas(jind);
        auto j = non_bas[jind];

        // Tab(qz+2,:) = Tab(qz+2,:)/Tab(qz+2,j);
        Tab1.row(qz + 2) /= Tab1(qz + 2, j);

        // row_ind = 1:size(Tab,1);
        // row_ind(qz+2) = [];
        u_rvector row_ind(Tab1.rows());
        std::iota(row_ind.begin(), row_ind.end(), 0);
        removeColumn(row_ind, qz + 2);

        // Tab(row_ind,:) = Tab(row_ind,:) - Tab(row_ind,j)*Tab(qz+2,:);
        Tab1(row_ind, Eigen::all) -= Tab1(row_ind, j) * Tab1(qz + 2, Eigen::all);

        // Bas(qz) = j;
        Bas1(qz) = j;
    }

    bool s = true;

    // if isempty(qz)
    if (qz < 0) // auxiliary z variable is non-basic and hence zero, feasible point reached
    {
        // qy = find(Bas == n+1, 1); % y must now be basic
        auto qy = find1(Bas1, equal_to(n));
        assert(qy >= 0);

        // Tab([1, qy+2],:) = [];
        removeRow(Tab1, qy + 2);
        removeRow(Tab1, 0);

        // Bas(qy) = [];
        removeColumn(Bas1, qy);

        // Tab(:,n+(1:2)) = [];
        removeColumn(Tab1, n + 1);
        removeColumn(Tab1, n + 0);
//         // or
//         removeColumn(Tab1, n);
//         removeColumn(Tab1, n);
    }
    else // z remained positive, there is no feasible point
    {
        s = false;
    }

    Tab.swap(Tab1);
    Bas.swap(Bas1);

    return s;
}

}
