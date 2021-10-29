// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Roland Hildebrand <roland.hildebrand@univ-grenoble-alpes.fr>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#include "parameters.hpp"

namespace mad_min
{

Parameters::Parameters(
    uint16_t W,
    uint16_t T,
    double bTcoef, // FIXME remove and pass beta ?
    double mu0,
    double wmin,
    const d_matrix& R,
    const d_cvector& Gr,
    const cvector<uint16_t>& numax
)
    : W(W)
    , T(T)
    , bTcoef(bTcoef)
    , mu0(mu0)
    , wmin(wmin)
    , R(R)
    , Gr(Gr)
    , rbar(W)
    , numax(numax)
{
    assert(R.rows() == T);
    assert(R.cols() == W);
    assert(Gr.size() == W);
    assert(numax.size() == W);

    // Eigen default storage is column-major, so we process our data by columns
    // TODO use row-major in the future ? - better performance?
    for (size_t i = 0; i < W; ++i) {
//         rbar[i] = R.col(i).mean();
        rbar[i] = R.col(i).sum() / R.rows();
    }

    // obj.M = R - ones(T,1) * rbar;
    M = R - d_cvector::Ones(T) * rbar;
//     print("M: {}x{}\n", M.rows(), M.cols());

    // [~,ind] = sort(-rbar./Gr');
    // obj.ind_rho_div_Gr = ind;
    ind_rho_div_Gr.resize(W);
    std::iota(ind_rho_div_Gr.begin(), ind_rho_div_Gr.end(), 0);
    std::stable_sort(
        ind_rho_div_Gr.begin(),
        ind_rho_div_Gr.end(),
        [this, Gr](size_t i1, size_t i2) {
            return (-rbar[i1] / Gr[i1]) < (-rbar[i2] / Gr[i2]);
    });

//     init_table_v1();
    init_table_v2();

//     auto TTT = init_tbl_v0(W, T, Gr, mu0, wmin, rbar, M);
//     auto delta = TTT - zeroTableau;
//     print("delta = {:e} {:e} {:e}\n", delta.norm(), delta.lpNorm<1>(), delta.lpNorm<Eigen::Infinity>());
//
//     for (int i = 0; i < TTT.rows(); ++i) {
//         auto delta = TTT.row(i) - zeroTableau.row(i);
//         if (delta.norm() > 0.0) {
//             print("{}\n", i);
//         }
//         for (int j = 0; j < TTT.cols(); ++j) {
//             if (TTT(i, j) != zeroTableau(i, j)) {
//                 print("{} {} : {:e} {:e}\n", i, j, TTT(i, j), zeroTableau(i, j));
//             }
//         }
//     }
}

void
Parameters::init_table_v1()
{
    const size_t ROWS = 2 * W + T + 4;
    const size_t COLS = 3 * W + 2 * T + 4;

    size_t col = COLS;
    size_t row = 0;
    size_t block_r_size = 0;

    zeroTableau = d_matrix::Zero(ROWS, COLS);

    auto add_blocks_row = [&col, &row, &block_r_size, COLS](const uint16_t r_size) {
        assert(col == COLS);
        col = 0;
        row += block_r_size;
        block_r_size = r_size;
    };

    auto add_block = [&col, &row, &block_r_size, this](const uint16_t c_size) {
        col += c_size;
        return zeroTableau.block(row, col - c_size, block_r_size, c_size);
    };

    const auto I_W = d_matrix::Identity(W, W);
    const auto I_T = d_matrix::Identity(T, T);

    // zeros(1,3*(numshares+1)+T), ones(1,T), 0; ...
    add_blocks_row(1);
    add_block(3 * (W + 1) + T)/* .setZero() */;
    add_block(T).setOnes();
    add_block(1)/* .setZero() */;

    // eye(numshares), -eye(numshares), zeros(numshares,numshares+4+2*T); ...
    add_blocks_row(W);
    add_block(W) = I_W;
    add_block(W) = -I_W;
    add_block(W + 4 + 2 * T)/* .setZero() */;

    // eye(numshares), zeros(numshares), eye(numshares), zeros(numshares,4+2*T); ...
    add_blocks_row(W);
    add_block(W) = I_W;
    add_block(W)/* .setZero() */;
    add_block(W) = I_W;
    add_block(4 + 2 * T)/* .setZero() */;

    // Gr', zeros(1,2*numshares), -1, zeros(1,2*(T+1)), wmin; ...
    add_blocks_row(1);
    add_block(W) = Gr.transpose();
    add_block(2 * W)/* .setZero() */;
    add_block(1).setConstant(-1.0);
    add_block(2 * (T + 1))/* .setZero() */;
    add_block(1).setConstant(wmin);

    // Gr', zeros(1,2*numshares+1), 1, zeros(1,1+2*T), 1; ...
    add_blocks_row(1);
    add_block(W) = Gr.transpose();
    add_block(2 * W + 1)/* .setZero() */;
    add_block(1).setOnes();
    add_block(2 * T + 1)/* .setZero() */;
    add_block(1).setOnes();

    // rbar, zeros(1,2*(numshares+1)), -1, zeros(1,2*T), mu0; ...
    add_blocks_row(1);
    add_block(W) = rbar;
    add_block(2 * (W + 1))/* .setZero() */;
    add_block(1).setConstant(-1.0);
    add_block(2 * T)/* .setZero() */;
    add_block(1).setConstant(mu0);

    // obj.M, zeros(T,2*numshares+3), -eye(T), eye(T), zeros(T,1)];
    add_blocks_row(T);
    add_block(W) = M;
    add_block(2 * W + 3)/* .setZero() */;
    add_block(T) = -I_T;
    add_block(T) = I_T;
    add_block(1)/* .setZero() */;

    add_blocks_row(0);
    assert(row == ROWS);
}

void
Parameters::init_table_v2()
{
    const size_t ROWS = 2 * W + T + 4;
    const size_t COLS = 3 * W + 2 * T + 4;

    zeroTableau = d_matrix(ROWS, COLS);
    zeroTableau = d_matrix(1 + W + W + 1 + 1 + 1 + T, COLS);

    const auto I_W = d_matrix::Identity(W, W);
    const auto I_T = d_matrix::Identity(T, T);

    zeroTableau <<
        // zeros(1,3*(numshares+1)+T), ones(1,T), 0; ...
        d_matrix::Zero(1, 3 * (W + 1) + T),
        d_matrix::Ones(1, T),
        0.0,

        // eye(numshares), -eye(numshares), zeros(numshares,numshares+4+2*T); ...
        I_W,
        -I_W,
        d_matrix::Zero(W, W + 4 + 2 * T),

        // eye(numshares), zeros(numshares), eye(numshares), zeros(numshares,4+2*T); ...
        I_W,
        d_matrix::Zero(W, W),
        I_W,
        d_matrix::Zero(W, 2 * T + 4),

        // Gr', zeros(1,2*numshares), -1, zeros(1,2*(T+1)), wmin; ...
        Gr.transpose(),
        d_matrix::Zero(1, 2 * W),
        -1.0,
        d_matrix::Zero(1, 2 * (T + 1)),
        wmin,

        // Gr', zeros(1,2*numshares+1), 1, zeros(1,1+2*T), 1; ...
        Gr.transpose(),
        d_matrix::Zero(1, 2 * W + 1),
        1.0,
        d_matrix::Zero(1, 2 * T + 1),
        1.0,

        // rbar, zeros(1,2*(numshares+1)), -1, zeros(1,2*T), mu0; ...
        rbar,
        d_matrix::Zero(1, 2 * (W + 1)),
        -1.0,
        d_matrix::Zero(1, 2 * T),
        mu0,

        // obj.M, zeros(T,2*numshares+3), -eye(T), eye(T), zeros(T,1)];
        M,
        d_matrix::Zero(T, 2 * W + 3),
        -I_T,
        I_T,
        d_matrix::Zero(T, 1);
}

d_matrix
Parameters::init_tbl_v0(
    const uint16_t W,
    const uint16_t T,
    const d_cvector& Gr,
    const double mu0,
    const double wmin,
    const d_rvector& rbar,
    const d_matrix& M
    )
{
    const size_t Y = 2 * W + T + 4;
    const size_t X = 3 * W + 2 * T + 4;

    d_matrix table = d_matrix::Zero(Y, X);

    size_t x = 0;
    size_t y = 0;
    size_t dx = 0;
    size_t dy = 0;

    // zeros(1,3*(numshares+1)+T), ones(1,T), 0; ...
    dy = 1;

    dx = 3 * (W + 1) + T;
    table.block(y, x, dy, dx).setConstant(0.0);
    x += dx;

    dx = T;
    table.block(y, x, dy, dx).setConstant(1.0);
    x += dx;

    dx = 1;
    table.block(y, x, dy, dx).setConstant(0.0);
    x += dx;

    assert(x == X);
    x = 0;
    y += dy;

    // eye(numshares), -eye(numshares), zeros(numshares,numshares+4+2*T); ...
    dy = W;

    dx = W;
    table.block(y, x, dy, dx) = d_matrix::Identity(dy, dx);
    x += dx;

    dx = W;
    table.block(y, x, dy, dx) = -d_matrix::Identity(dy, dx);
    x += dx;

    dx = W + 4 + 2 * T;
    table.block(y, x, dy, dx).setConstant(0.0);
    x += dx;

    assert(x == X);
    x = 0;
    y += dy;

    // eye(numshares), zeros(numshares), eye(numshares), zeros(numshares,4+2*T); ...
    dy = W;

    dx = W;
    table.block(y, x, dy, dx) = d_matrix::Identity(dy, dx);
    x += dx;

    dx = W;
    table.block(y, x, dy, dx).setConstant(0.0);
    x += dx;

    dx = W;
    table.block(y, x, dy, dx) = d_matrix::Identity(dy, dx);
    x += dx;

    dx = 4 + 2 * T;
    table.block(y, x, dy, dx).setConstant(0.0);
    x += dx;

    assert(x == X);
    x = 0;
    y += dy;

    // Gr', zeros(1,2*numshares), -1, zeros(1,2*(T+1)), wmin; ...
    dy = 1;

    dx = W;
    table.block(y, x, dy, dx) = Gr.transpose();
    x += dx;

    dx = 2 * W;
    table.block(y, x, dy, dx).setConstant(0.0);
    x += dx;

    dx = 1;
    table.block(y, x, dy, dx).setConstant(-1.0);
    x += dx;

    dx = 2 * (T + 1);
    table.block(y, x, dy, dx).setConstant(0.0);
    x += dx;

    dx = 1;
    table.block(y, x, dy, dx).setConstant(wmin);
    x += dx;

    assert(x == X);
    x = 0;
    y += dy;

    // Gr', zeros(1,2*numshares+1), 1, zeros(1,1+2*T), 1; ...
    dy = 1;

    dx = W;
    table.block(y, x, dy, dx) = Gr.transpose();
    x += dx;

    dx = 2 * W + 1;
    table.block(y, x, dy, dx).setConstant(0.0);
    x += dx;

    dx = 1;
    table.block(y, x, dy, dx).setConstant(1.0);
    x += dx;

    dx = 2 * T + 1;
    table.block(y, x, dy, dx).setConstant(0.0);
    x += dx;

    dx = 1;
    table.block(y, x, dy, dx).setConstant(1.0);
    x += dx;

    assert(x == X);
    x = 0;
    y += dy;

    // rbar, zeros(1,2*(numshares+1)), -1, zeros(1,2*T), mu0; ...
    dy = 1;

    dx = W;
    table.block(y, x, dy, dx) = rbar;
    x += dx;

    dx = 2 * (W + 1);
    table.block(y, x, dy, dx).setConstant(0.0);
    x += dx;

    dx = 1;
    table.block(y, x, dy, dx).setConstant(-1.0);
    x += dx;

    dx = 2 * T;
    table.block(y, x, dy, dx).setConstant(0.0);
    x += dx;

    dx = 1;
    table.block(y, x, dy, dx).setConstant(mu0);
    x += dx;

    assert(x == X);
    x = 0;
    y += dy;

    // obj.M, zeros(T,2*numshares+3), -eye(T), eye(T), zeros(T,1)];
    dy = T;

    dx = W;
    table.block(y, x, dy, dx) = M;
    x += dx;

    dx = 2 * W + 3;
    table.block(y, x, dy, dx).setConstant(0.0);
    x += dx;

    dx = T;
    table.block(y, x, dy, dx) = -d_matrix::Identity(dy, dx);
    x += dx;

    dx = T;
    table.block(y, x, dy, dx) = d_matrix::Identity(dy, dx);
    x += dx;

    dx = 1;
    table.block(y, x, dy, dx).setConstant(0.0);
    x += dx;

    assert(x == X);
    x = 0;
    y += dy;

    assert(y == Y);

    return table;
}

}
