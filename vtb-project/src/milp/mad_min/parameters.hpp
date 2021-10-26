// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Roland Hildebrand <roland.hildebrand@univ-grenoble-alpes.fr>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#pragma once

#include "common.hpp"

namespace mad_min
{

struct Parameters
{
    using Ptr = std::shared_ptr<Parameters>;

    /// number of shares
    uint16_t W;

    /// length of history
    uint16_t T;

    /// bTcoef = 1/((1-beta)*T)
    double bTcoef;

    /// mu0 lower bound on returns
    double mu0;

    /// min weight of shares in capital
    double wmin;

    /// the returns, multiplied by the granularities, T x W
    d_matrix R;

    /// the granularities, column vector, W x 1
    d_cvector Gr;

    /// rbar are the means over t of R, a row vector, 1 x W
    d_rvector rbar;

    /// maximal number of lots for each share, integer column vector, W x 1
    cvector<uint16_t> numax;

    /// % for MAD: M = R - ones(T,1)*rbar, matrix in cost function for MADmin
    d_matrix M;

    /// for auxiliary fast LP: sorted index list of ratio -rbar/Gr
    // 1 x W
    u_rvector ind_rho_div_Gr;

    /// unnormalized simplex tableau coding the LP in the nodes
    d_matrix zeroTableau;

    Parameters(
        uint16_t W,
        uint16_t T,
        double bTcoef, // FIXME remove and pass beta ?
        double mu0,
        double wmin,
        const d_matrix& R,
        const d_cvector& Gr,
        const cvector<uint16_t>& numax
    );

    Parameters(const Parameters& other) = default;

private:
    void
    init_table_v2();

    // FIXME remove
    void
    init_table_v1();

    // FIXME remove
    d_matrix
    init_tbl_v0(
        const uint16_t W,
        const uint16_t T,
        const d_cvector& Gr,
        const double mu0,
        const double wmin,
        const d_rvector& rbar,
        const d_matrix& M
    );

#if defined HDF5_TESTS
public:
    void
    h5_save(const std::string& file_name, std::string_view prefix = {});

    static
    Parameters::Ptr
    h5_load(const std::string& file_name, int row);

    static
    Parameters::Ptr
    mat_load(std::string_view file_name, int row, std::string_view prefix = {});

    bool
    mat_test(std::string_view file_name, int row, std::string_view prefix = {});

// private:
    Parameters() {};
#endif
};

}
