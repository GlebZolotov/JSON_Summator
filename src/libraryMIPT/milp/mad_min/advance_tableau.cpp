// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Roland Hildebrand <roland.hildebrand@univ-grenoble-alpes.fr>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#include "utils.hpp"

#include "advance_tableau.hpp"

namespace mad_min
{

// FIXME
double tt1 = 0.0;
double tt2 = 0.0;
double tt3 = 0.0;
double tt4 = 0.0;

std::tuple<bool /* opt */, bool /* ub */>
advance_tableau(
    d_matrix& T, // table
    u_rvector& B, // basis
    bool ph1)
{
    bool opt = false;
    bool ub = false;
    Timer tm;

    // FAST, manual version is equal by speed
    tm.restart();
    // [mi,j] = min(T(1,1:end-1));
    const auto [mi, j] = min(T.row(0).leftCols(T.cols() - 1));
    tt1 += tm.ms();

    if (mi >= -1e-14) // FIXME magic constant
    {
        // program is at optimal point
        opt = true;

        return { opt, ub };
    }

    // j is the index of the entering basic column
    // if ph1
    //     astart = 3;
    // else
    //     astart = 2;
    // end
    const auto astart = (ph1) ? 2 : 1;

//     // ==========================
//     // v0
//     // ==========================
//     tm.restart();
//     // posind = find(T(astart:end,j) > 0) + astart - 1;
// //     // v0.1
// //     auto posind = find(T.col(j).bottomRows(T.rows() - astart),
// //                        [](double x) { return x > 0; });
// //     for (auto &i : posind) {
// //         i += astart;
// //     }
//
//     // v0.2 ~ 15% speedup for v0.tt2
//     static std::vector<Eigen::Index> posind;
//     posind.reserve(1000);
//     find(T.col(j).bottomRows(T.rows() - astart),
//          [](double x) { return x > 0; },
//          posind);
//
//     for (auto &i : posind) {
//         i += astart;
//     }
//
//     if (posind.empty()) {
//         // problem unbounded
//         ub = true;
//
//         return { opt, ub };
//     }
//
//     // baratio = T(posind,end)./T(posind,j);
//     auto baratio = T(posind, T.cols() - 1).array() / T(posind, j).array();
//
//     // q is the pivot row
//     // [~,minind] = min(baratio);
//     // q = posind(minind);
//     auto q = posind[min_idx(baratio)];
//     tt2 += tm.ms();
//     // ==========================

    // ==========================
    // v1 > 50% speedup for v0.tt2
    // ==========================
    // posind = find(T(astart:end,j) > 0) + astart - 1;
    // if isempty(posind) ...
    // baratio = T(posind,end)./T(posind,j);
    // [~,minind] = min(baratio);
    // q = posind(minind);
    tm.restart();
    // no effect ?
    Eigen::Index q = -1;
    double baratio_min = d_inf;
    for (Eigen::Index i = astart; i < T.rows(); ++i)
    {
        if (T(i, j) > 0.0)
        {
            auto baratio = T(i, T.cols() - 1) / T(i, j);
            if (baratio < baratio_min)
            {
                baratio_min = baratio;
                q = i;
            }
        }
    }
    tt2 += tm.ms();
    // ==========================

    if (q < 0)
    {
        // problem unbounded
        ub = true;

        return { opt, ub };
    }

    // i = B(q - astart + 1) is the leaving basic column
    // B(q - astart + 1) = j;
    B(q - astart) = j;

    tm.restart();
    // T(q,:) = T(q,:)/T(q,j);
    T.row(q) /= T(q, j);
    tt3 += tm.ms();

    // ind = 1:size(T,1);
    // ind(q) = [];
    // T(ind,:) = T(ind,:) - T(ind,j)*T(q,:);

//     // ==========================
//     // v0 - TOOOOO SLOOOOOWWWWW
//     tm.restart();
//     static std::vector<uint16_t> ind;
//     ind.reserve(1000);
//     ind.clear();
//     for (uint16_t i = 0; i < T.rows(); ++i) {
//         if (i == q) {
//             continue;
//         }
//         ind.push_back(i);
//     }
//     T(ind, Eigen::all) -= T(ind, j) * T.row(q);
//     tt4 += tm.ms();
//     // ==========================

    // ==========================
    // v1 > 350% speedup for v0.tt4
    tm.restart();
    for (uint16_t i = 0; i < T.rows(); ++i) {
        if (i == q) {
            continue;
        }
        T.row(i) -= T(i, j) * T.row(q);
    }
    tt4 += tm.ms();
    // ==========================

    return { opt, ub };
}

}
