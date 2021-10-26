// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Roland Hildebrand <roland.hildebrand@univ-grenoble-alpes.fr>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#include "integer_local_improve.hpp"

namespace mad_min
{

template <typename T>
T
min(const T& v)
{
    return v;
}

template <typename T, typename ...Ts>
T
min(const T& v1, const T& v2, Ts... args)
{
    if (v1 < v2)
    {
        return min(v1, args...);
    }
    else
    {
        return min(v2, args...);
    }
}

std::tuple<bool /*s*/, double /*value*/, d_cvector /*nu*/>
integer_local_improve(
    const d_rvector& rbar,
    const d_cvector& Gr,
    const d_matrix& M,
    uint16_t W,
    const u_cvector& numax,
    double mu0,
    double wmin,
    const d_cvector& nu)
{
    // nu = double(nu);
    auto nu1 = nu;

    // muslack = commonParameters.rbar*nu - commonParameters.mu0;
    double muslack = rbar.dot(nu1) - mu0;

    // upperwslack = 1 - nu'*commonParameters.Gr;
    double upperwslack = 1.0 - nu1.dot(Gr);

    // lowerwslack = nu'*commonParameters.Gr - commonParameters.wmin;
    double lowerwslack = nu1.dot(Gr) - wmin;

    // FIXME remove and test each value separately after creation
    // if min([muslack,upperwslack,lowerwslack]) < 0
    if (min(muslack, upperwslack, lowerwslack) < 0.0)
    {
        return { false, 0.0, nu };
    }

    // start local descent
    // ab = commonParameters.M*nu;
    // value = sum(abs(ab))/2;
    d_cvector ab = M * nu1;
    double value = 0.5 * ab.cwiseAbs().sum();

    // there are 2n candidate steps, (+delta(i),-delta(i))
    // J = [eye(commonParameters.numshares), -eye(commonParameters.numshares)];
    d_matrix J(W, W * 2);
    J << d_matrix::Identity(W, W), -d_matrix::Identity(W, W);

    while (true)
    {
        // find variations not violating feasibility
        // feas_ind = find(
        //   ([nu'+1-double(commonParameters.numax)',1-nu'] <= 0) &
        //   ([-commonParameters.rbar,commonParameters.rbar] <= muslack) &
        //   ([commonParameters.Gr',-commonParameters.Gr'] <= upperwslack) &
        //   ([-commonParameters.Gr',commonParameters.Gr'] <= lowerwslack));

        std::vector<Eigen::Index> feas_ind;
        for (Eigen::Index i = 0; i < W; ++i)
        {
            //  nu'+1-double(commonParameters.numax)' <= 0 &
            // -commonParameters.rbar <= muslack &
            //  commonParameters.Gr' <= upperwslack &
            // -commonParameters.Gr' <= lowerwslack
            if (
                ((nu1[i] + 1 - numax[i]) <= 0) &&
                (-rbar[i] <= muslack) &&
                (Gr[i] <= upperwslack) &&
                (-Gr[i] <= lowerwslack)
            )
            {
                feas_ind.push_back(i);
            }

            //  1-nu' <= 0 &
            //  commonParameters.rbar <= muslack &
            // -commonParameters.Gr' <= upperwslack &
            //  commonParameters.Gr' <= lowerwslack
            if (
                ((1 - nu1[i]) <= 0) &&
                (rbar[i] <= muslack) &&
                (-Gr[i] <= upperwslack) &&
                (Gr[i] <= lowerwslack)
            )
            {
                feas_ind.push_back(W + i);
            }
        }

        if (feas_ind.empty())
        {
            break;
        }

        // nucandidates = nu*ones(1,length(feas_ind)) + J(:,feas_ind);
        d_matrix nucandidates = nu1 * d_matrix::Ones(1, feas_ind.size()) + J(Eigen::all, feas_ind);

        // L1vec = ab*ones(1,length(feas_ind)) + commonParameters.M*J(:,feas_ind);
        d_matrix L1vec = ab * d_matrix::Ones(1, feas_ind.size()) + M * J(Eigen::all, feas_ind);

        // [mi,minind] = min(sum(abs(L1vec))/2);
        Eigen::Index minind;
        double mi = 0.5 * L1vec.cwiseAbs().colwise().sum().minCoeff(&minind);

        if (mi < value)
        {
            value = mi;

            // ind = feas_ind(minind);
            Eigen::Index ind = feas_ind[minind];

            // nu = nucandidates(:,minind);
            nu1 = nucandidates(Eigen::all, minind);

            ab = L1vec(Eigen::all, minind);

            if (ind < W)
            {
                // muslack = muslack + commonParameters.rbar(ind);
                muslack += rbar[ind];

                // upperwslack = upperwslack - commonParameters.Gr(ind);
                upperwslack -= Gr[ind];

                // lowerwslack = lowerwslack + commonParameters.Gr(ind);
                lowerwslack += Gr[ind];
            }
            else
            {
                ind -= W;

                // muslack = muslack - commonParameters.rbar(ind-double(commonParameters.numshares));
                muslack -= rbar[ind];

                // upperwslack = upperwslack + commonParameters.Gr(ind-double(commonParameters.numshares));
                upperwslack += Gr[ind];

                // lowerwslack = lowerwslack - commonParameters.Gr(ind-double(commonParameters.numshares));
                lowerwslack -= Gr[ind];
            }
        }
        else
        {
            break;
        }
    }

    return { true, value, nu1 };
}

}
