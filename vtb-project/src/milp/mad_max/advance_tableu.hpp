#pragma once
#include <Eigen/Dense>
#include "types.hpp"

struct AdvanceTable_Result {
    bool opt;
    bool ub;
};

AdvanceTable_Result advance_tableu(Eigen::MatrixXd& T, RowVectorXi32& B, bool ph1);