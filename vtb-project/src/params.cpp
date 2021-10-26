#include "params.hpp"
#include "./io/csv_reader.hpp"
#include "utils.hpp"

#include <limits>
#include <cassert>
#include <iostream>
//Asserts floating point compatibility at compile time
static_assert(std::numeric_limits<float>::is_iec559, "IEEE 754 required");
double double_inf = std::numeric_limits<double>::infinity();
double negative_double_inf = std::numeric_limits<double>::infinity();

Eigen::RowVectorXi getRandomIntRowVec(size_t size, int start, int end)
{
    std::set<int> randomSet;
    while (randomSet.size() < size)
        randomSet.insert(Utils::random(start, end));

    Eigen::RowVectorXi row(randomSet.size());
    size_t i = 0;
    for (auto it = randomSet.begin(); it != randomSet.end(); it++)
        row[i++] = *it;
    return row;
}

std::string data_path_s = "../data/sharedata_reduced.csv";
void CommonParams::setRandomParams()
{
    numShares = 100;//Utils::random<int>(100, 200); 
    ind_shares = getRandomIntRowVec(numShares, 0, 199); 
    p_max = Utils::random<float>(0.1, 0.3);
    T = Utils::random<int>(100, 300);
    w_min = Utils::random<double>(0.9, 0.99);
    beta = 0.95;
    bT_coef = 1/((1-beta)*T); 
    tolInt = pow(10, -6); 

    std::ifstream file(data_path_s);
    CSVRow row(file);
    int i = 0;
    Gr.resize(numShares);
    R.resize(T, numShares);
    while (file >> row && i < ind_shares.size())
    {
        int index = ind_shares[i];
        int row_ind = std::stoi(row["Index"]);
        if (row_ind == index)
        {
            Gr[i] = std::stod(row["Gran"]);

            for (int j = 0; j < T; j++)
            {
                std::string key = "RetG_" + std::to_string(j);
                R(j, i) = std::stod(row[key]);
            }
            i++;
        }
    }
    rbar = R.colwise().mean();
    nu_max = floor((VectorXd::Ones(numShares) * p_max).cwiseQuotient(Gr).array()).cast<int32_t>();

    indGr.resize(Gr.size());
    std::size_t n(0); 
    std::generate(indGr.begin(), indGr.end(), [&]
                  { return n++; });
    std::sort(indGr.begin(), indGr.end(),
              [&](int i1, int i2)
              { return Gr[i1] >= Gr[i2]; });
}

CommonParams_MADmax::CommonParams_MADmax(CommonParams *common_params)
{
    params = common_params;
    gamma_mad = Utils::random<float>(0.003, 0.02);
    M.resize(params->T, params->numShares);
    M = params->R - Eigen::MatrixXd::Ones(params->T, 1) * params->rbar;

    std::vector<int> ind_rho_div_Gr_vec;
    ind_rho_div_Gr_vec.resize(params->numShares);
    std::iota(ind_rho_div_Gr_vec.begin(), ind_rho_div_Gr_vec.end(), 0);
    std::stable_sort(
        ind_rho_div_Gr_vec.begin(),
        ind_rho_div_Gr_vec.end(),
        [this](size_t i1, size_t i2)
        {
            return (-params->rbar(i1) / params->Gr(i1)) < (-params->rbar(i2) / params->Gr(i2));
        });
    ind_rho_div_Gr = Eigen::Map<Eigen::VectorXi>(ind_rho_div_Gr_vec.data(), ind_rho_div_Gr_vec.size());

    size_t N = params->numShares;
    size_t T = params->T;

    size_t rows = 2 * N + T + 5; //ERROR? 2*N + T + 4
    size_t cols = 3 * N + 2 * T + 4;
    zeroTableau.resize(rows, cols);
    zeroTableau.block(0, 0, 1, cols) << RowVectorXd::Zero(3 * N + 2), 1, RowVectorXd::Zero(2 * T + 1);
    zeroTableau.block(1, 0, 1, cols) << -params->rbar, RowVectorXd::Zero(2 * (N + T) + 4);
    zeroTableau.block(2, 0, N, cols) << MatrixXd::Identity(N, N), (-1) * MatrixXd::Identity(N, N), MatrixXd::Zero(N, N + 4 + 2 * T);
    zeroTableau.block(2 + N, 0, N, cols) << MatrixXd::Identity(N, N), MatrixXd::Zero(N, N), MatrixXd::Identity(N, N), MatrixXd::Zero(N, 4 + 2 * T);
    zeroTableau.block(2 + 2 * N, 0, 1, cols) << params->Gr.transpose(), RowVectorXd::Zero(2 * N), -1, RowVectorXd::Zero(2 * (T + 1)), params->w_min;
    zeroTableau.block(3 + 2 * N, 0, 1, cols) << params->Gr.transpose(), RowVectorXd::Zero(2 * N + 1), 1, RowVectorXd::Zero(1 + 2 * T), 1;
    zeroTableau.block(4 + 2 * N, 0, 1, cols) << RowVectorXd::Zero(3 * N + 2), -1, RowVectorXd::Zero(T), RowVectorXd::Ones(T), gamma_mad * T / 2;
    zeroTableau.block(5 + 2 * N, 0, T, cols) << M, MatrixXd::Zero(T, 2 * N + 3), (-1) * MatrixXd::Identity(T, T), MatrixXd::Identity(T, T), MatrixXd::Zero(T, 1);
}

CommonParams_MADmax::~CommonParams_MADmax()
{
    delete params;
}

CommonParams_MADmin::CommonParams_MADmin()
{
    params = new CommonParams();
}

CommonParams_MADmin::~CommonParams_MADmin()
{
    delete params;
}

double computeMADvariance(CommonParams_MADmax *max_params, Eigen::VectorXd nu)
{
    return (max_params->M * nu).cwiseAbs().sum() / (max_params->params->T);
}