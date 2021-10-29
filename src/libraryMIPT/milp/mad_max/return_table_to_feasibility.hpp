#pragma once
#include <Eigen/Dense>
#include "types.hpp"

//adapts a simplex table to a change in the right-hand side of a linear
//equation system with m equations and n variables
//Tab in R^((m+1) x (n+1)) is the table to be corrected, Bas in R^m the basic set
//delta_slack in R^n is the column vector of additive changes in the variables
//negative entries mean the corresponding constraints are tightened,
//positive entries mean the constraints are relaxed
//s is a boolean indicating whether a feasible point has been found with the new bounds
//we introduce two auxiliary variables y,z >= 0, y+z = 1
//at the current point y = 0, z = 1, z added to basic set
//at the sought feasible point y = 1, z = 0
bool return_table_to_feasibility(Eigen::VectorXd delta_slack, Eigen::MatrixXd& Tab, RowVectorXi32 Bas);