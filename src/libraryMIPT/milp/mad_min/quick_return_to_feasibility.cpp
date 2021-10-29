// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Roland Hildebrand <roland.hildebrand@univ-grenoble-alpes.fr>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#include "advance_tableau.hpp"
#include "utils.hpp"

#include "quick_return_to_feasibility.hpp"

namespace mad_min
{

bool
quick_return_to_feasibility(
    double delta,
    uint16_t j,
    uint16_t q,
    d_matrix& Tab,
    u_rvector& Bas,
    bool verbose)
{
    bool s = true;

    // Tab(q+1,end) = Tab(q+1,end) + delta;
    Tab(q + 1, cend(Tab)) += delta;

    // if Tab(q+1,end) < 0
    if (Tab(q + 1, cend(Tab)) < 0.0)
    {
        // Tab(q+1,:) = -Tab(q+1,:);
        Tab.row(q + 1) *= -1.0;

        // Tab(:,j) = -Tab(:,j);
        Tab.col(j) *= -1.0;

        // adding row corresponding to auxiliary cost function
        // Tab = [-Tab(q+1,:); Tab];
        d_matrix Tab2(Tab.rows() + 1, Tab.cols());
        Tab2 << -Tab.row(q + 1), Tab;

        // Tab(1,j) = 0;
        Tab2(0, j) = 0.0;

        // solving auxiliary program
        int count_advance = 0;
        Timer tm;
        while (true)
        {
            ++count_advance;
            auto [opt, ub] = advance_tableau(Tab2, Bas, true);
            if (opt)
            {
                break;
            }
        }

        if (verbose)
        {
            const double t = tm.ms();
            print(
                "QR Phase 1 from parent table: {:4} simplex iterations in {:8.2f} ms; "
                "{:8.4f} ms/it; {:8.2f} it/s\n",
                count_advance, t,
                t / count_advance, count_advance / t * 1000.0);
        }

        // if Tab(1,end) < -10^(-12)
        if (Tab2(0, cend(Tab2)) < -1e-12)
        {
            s = false;
        }
        else
        {
            // Tab(1,:) = [];
            removeRow(Tab2, 0);

            // Tab(:,j) = -Tab(:,j);
            Tab2.col(j) *= -1.0;

            // q = find(Bas == j,1);
            auto q = find1(Bas, equal_to(j));

            // if ~isempty(q)
            if (q >= 0) // variable j is still basic
            {
                // Tab(q+1,:) = -Tab(q+1,:);
                Tab2.row(q + 1) *= -1.0;

                // Tab(q+1,end) = 0;
                Tab2(q + 1, cend(Tab2)) = 0.0;
            }
        }

        Tab.swap(Tab2);
    }

    return s;
}

}
