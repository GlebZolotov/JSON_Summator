#pragma once

#include <vector>
#include <set>

#include "types.hpp"

extern double double_inf;
extern double negative_double_inf;

Eigen::RowVectorXi getRandomIntRowVec(size_t size, int start, int end);

struct CommonParams {

    int numShares; //Utils::random<int>(100, 200); //number of shares
    Eigen::RowVectorXi ind_shares; //indices of selected shares
    float p_max; //maximal weight of each share
    int T; //length of history, in [100,300]
    double w_min; //min weight of shares in capital, in [0.9,0.99]
    double beta; //confidence level
    Eigen::MatrixXd R; //the returns, multiplied by the granularities
    Eigen::VectorXd Gr; //the granularities, column vector
    Eigen::VectorXi indGr; //the indices of Gr sorted in descending order, column vector
    Eigen::RowVectorXd rbar; //the means over t of R, a row vector
    VectorXi32 nu_max; //maximal number of lots for each share, integer column vector
    double bT_coef; //for Cvar: bTcoef = 1/((1-beta)*T), coefficient at sum z_t in cost function
    double tolInt; //tolerance for recognizing a real value to be an integer

    CommonParams() = default;
    void setRandomParams();
};

struct CommonParams_MADmax
{
    CommonParams* params; //pointer to common params
    double gamma_mad; //maximal MAD variance in [0.003,0.02]
    Eigen::MatrixXd M; //matrix in cost function for MADmax
    Eigen::VectorXi ind_rho_div_Gr; //for auxiliary fast LP: sorted index list of ratio -rbar/Gr
    Eigen::MatrixXd zeroTableau; //unnormalized simplex tableau coding the LP in the nodes

    CommonParams_MADmax(CommonParams*);
    ~CommonParams_MADmax();
};

struct CommonParams_MADmin
{
    CommonParams* params;
    
    float mu_0; //lower bound on returns, in [0.0001,0.001]

    CommonParams_MADmin();
    ~CommonParams_MADmin();
};

double computeMADvariance(CommonParams_MADmax* params, Eigen::VectorXd nu);
