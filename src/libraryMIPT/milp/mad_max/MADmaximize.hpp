#pragma once

#include <Eigen/Dense>
#include "types.hpp"

using namespace Eigen;

struct CommonParams_MADmax;
class Node;

struct ReturnValues_maximize {
    bool s; //is status of infeasibility
    double value; //is the optimal value
    VectorXi32 nu_low; //is the lower nearest integer vectors to the optimal nu
    VectorXi32 nu_upp; //is the upper nearest integer vectors to the optimal nu
    VectorXd nu; //optimal vector value
    uint index = -1; //is the number of the fractional element (0 if there is none) 
    uint min_max = 0; //is a variable in {-1,0,+1} indicating whether the minimal value
    uint index1 = 0; //is the number of index in the sorted list ind_rho_div_Gr
};

struct ReturnValues_MADmaximize {
    bool s; //is status of infeasibility
    bool single_index; // is a boolean indicating whether the upper and lower bounds differ by at most one index (e.g., if the constraint 1/T*sum(abs(M*nu)) <= gammaMAD is not active)
    double value; //is the optimal value
    VectorXi32 nu_low; //is the lower nearest integer vectors to the optimal nu
    VectorXi32 nu_upp; //is the upper nearest integer vectors to the optimal nu
    VectorXd nu; //optimal vector value
    uint index; //is the number of the fractional element (0 if there is none) 
    Eigen::MatrixXd Tab; //is the optimal simplex tableau of the LP
    RowVectorXi32 Bas; //the corresponding basic index set node is the node which calls the function
};

//quick routine which maximizes rbar*nu over the cube {nu: nu_lower <= nu <= nu_upper} under
//the constraints wmin <= nu'*Gr <= 1
//dual variable is zero if unconditional maximum satisfies the constraints
//dual variable is positive or negative in dependence on whether the lower
//or upper constraint is violated
//in this case the dual variable to the constraints on nu'*Gr equals -rbar_i/Gr_i for
//some critical index i
ReturnValues_maximize maximize_return_on_cube(CommonParams_MADmax*, VectorXi32&, VectorXi32&);

//maximizes rbar*nu over the cube {nu: nu_lower <= nu <= nu_upper} under
//the constraints wmin <= nu'*Gr <= 1 and 1/T*sum(abs(M*nu)) <= gammaMAD
//boolean indicating whether the problem is feasible
ReturnValues_MADmaximize MADmaximize_return_on_cube(CommonParams_MADmax*, VectorXi32&, VectorXi32&, Node*);