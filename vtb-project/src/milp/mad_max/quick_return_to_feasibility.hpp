#pragma once
#include <Eigen/Dense>
#include "types.hpp"

//in the linear system corresponding to the simplex table Tab the basic
//variable j, corresponding to row q, has been changed by delta
//this can be corrected by adjusting the right-hand side of equation q
//if the table becomes infeasible, then a phase 1 like optimization is
//performed to return slack j to zero
//s is a boolean indicating whether a feasible point has been reached
bool quick_return_to_feasibility(double delta, int j, int q, Eigen::MatrixXd& Tab, RowVectorXi32& Bas);