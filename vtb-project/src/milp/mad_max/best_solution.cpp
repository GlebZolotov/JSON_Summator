#include "best_solution.hpp"

#include <cstdint>
#include <cmath>

BestIntegerSolution::BestIntegerSolution(double opt_value, VectorXi32 nu) 
{
    this->setValues(opt_value, nu);
}

void BestIntegerSolution::setValues(double opt_value, VectorXi32 nu) {
        upperBound = opt_value;
        if (!std::isinf(opt_value)) {
            bestNu = nu;
        }
}