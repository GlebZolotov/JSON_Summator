#include <iostream>
#include "params.hpp"
#include "./milp/mad_statement.hpp"
#include "./LP/simplex_method.hpp"
#include "./LP/lp_solver.hpp"
#include "./LP/mad_max_lp_solver.hpp"

int main() {
    CommonParams params;
    params.setRandomParams();
    CommonParams_MADmax* commonParametersMax = new CommonParams_MADmax(&params);
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
    LpSolver lpSolver(&linearSolver);
    MadMaxLpSolver solver(statement, &lpSolver);
    std::cout << "ROWS: " << statement->timeRate.rows() << ". COLS: " << statement->timeRate.cols() << std::endl;
    solver.Solve();
    MadSolution solution = solver.Current();
    std::cout << "DURATION: " << solution.time << std::endl;
}