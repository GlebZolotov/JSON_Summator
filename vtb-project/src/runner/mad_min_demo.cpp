#include <iostream>
#include "params.hpp"
#include "./milp/mad_statement.hpp"
#include "./lp/simplex_method.hpp"
#include "./milp/mad_min_solver.hpp"

int main() {
    CommonParams_MADmax* commonParametersMax = new CommonParams_MADmax();
    MadStatement * statement = new MadStatement();
    statement->timeRate = commonParametersMax->params->R.transpose();
    statement->meanRate = commonParametersMax->params->rbar.transpose();
    statement->maxWeight = Eigen::VectorXd::Constant(statement->timeRate.rows(), commonParametersMax->params->p_max); // nu_max
    statement->minReturn = 1;// Eigen::VectorXd::Constant(statement->timeRate.rows(), commonParametersMax->params->p_max);
    statement->maxRisk = commonParametersMax->gamma_mad;
    statement->lotSize = commonParametersMax->params->ind_shares;
    statement->close = Eigen::VectorXd::Constant(statement->timeRate.rows(), 1);
    statement->capital = 1000000;
    statement->minSize = commonParametersMax->params->w_min;
    statement->maxSize = 1;
    statement->objectiveUserLimit = -1000;
    statement->iterationLimit = 1000000;
    statement->nodeLimit = 10;
    statement->timeLimit = 100000;
    double threshold = 1e-10;
    SimplexMethod linearSolver(threshold);
    MadMinSolver solver(statement, threshold, &linearSolver);
    std::cout << "ROWS: " << statement->timeRate.rows() << ". COLS: " << statement->timeRate.cols() << std::endl;
    madMinLpSolver.Solve();
    MadSolution solution = madMinLpSolver.Current();
    std::cout << "DURATION: " << solution.time << std::endl;
}