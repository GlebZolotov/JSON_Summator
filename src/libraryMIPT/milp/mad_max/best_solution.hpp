#pragma once

#include <Eigen/Dense>

#include "types.hpp"

struct BestIntegerSolution {
    double upperBound;
    VectorXi32 bestNu; //best nu integer vector

    BestIntegerSolution(double opt_value, VectorXi32 nu);

    void setValues(double opt_value, VectorXi32 nu);
};
