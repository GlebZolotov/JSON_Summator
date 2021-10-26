#pragma once

#include "types.hpp"

struct CommonParams_MADmax;

//value is the maximal achieved objective value
//nu is an integer vector of the solution variables
//s is a boolean indicating whether a feasible solution has been found first check feasibility
struct ReturnValue_localImprove {
    bool s;
    VectorXi32 nu_local;
    double value;
};

//function checks its feasibility
//if it is feasible, greedy local descent is tried by changing entries by +-1
//if the solution cannot be further improved, the corresponding variables are given back
ReturnValue_localImprove MADmax_integer_local_improve(CommonParams_MADmax*, VectorXi32); //or VectorXd ?