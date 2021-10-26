#include "mad_max_lp_solver.hpp"

#include <Eigen/Dense>
#include <vector>
#include <time.h>

MadMaxLpSolver::MadMaxLpSolver(MadStatement * statement, LpSolver * linearSolver) : m_statement(statement)
{
    m_solver = linearSolver;
}

MadMaxLpSolver::~MadMaxLpSolver() {

}

MadSolution & MadMaxLpSolver::Solve() {
    Eigen::MatrixXd constraintMatrix = CreateConstraintMatrix();
    Eigen::VectorXd constraintVector = CreateConstraintVector();
    Eigen::VectorXd objectiveVector = CreateObjectiveVector();
    int iterations = m_statement->iterationLimit;
    m_solver->Solve(objectiveVector, constraintMatrix, constraintVector, iterations);
    return Current();
}

void MadMaxLpSolver::Interrupt() {
    m_solver->Interrupt();
}

Eigen::MatrixXd MadMaxLpSolver::CreateConstraintMatrix() {
    int rows = m_statement->timeRate.rows();
    int cols = m_statement->timeRate.cols();
    Eigen::MatrixXd constraintMatrix(cols + 1 + 1 + rows, rows + cols * 2 + 1 + rows);
    Eigen::MatrixXd rateMatrix = m_statement->timeRate.colwise() - m_statement->meanRate;
    constraintMatrix.block(0,0,cols, rows + cols * 2) << rateMatrix.transpose(), -Eigen::MatrixXd::Identity(cols, cols), Eigen::MatrixXd::Identity(cols, cols);
    constraintMatrix.block(cols + 0, 0, 1, rows + cols * 2) << Eigen::RowVectorXd::Zero(rows + cols), Eigen::RowVectorXd::Constant(cols, 2/(cols));
    constraintMatrix.block(cols + 1, 0, 1, rows + cols * 2) << Eigen::RowVectorXd::Constant(rows, 1.0), Eigen::RowVectorXd::Zero(cols + cols);
    constraintMatrix.block(cols + 2, 0, rows, rows + cols * 2) << Eigen::MatrixXd::Identity(rows,rows), Eigen::MatrixXd::Zero(rows, cols * 2);

    constraintMatrix.block(0, rows + cols * 2, cols, 1 + rows) << Eigen::MatrixXd::Zero(cols, 1 + rows);
    constraintMatrix.block(cols + 0, rows + cols * 2, 1, 1 + rows) << 1.0, Eigen::RowVectorXd::Zero(rows);
    constraintMatrix.block(cols + 1, rows + cols * 2, 1, 1 + rows) << Eigen::RowVectorXd::Zero(1 + rows);
    constraintMatrix.block(cols + 2, rows + cols * 2, rows, 1) << Eigen::VectorXd::Zero(rows, 1);
    constraintMatrix.block(cols + 2, rows + cols * 2 + 1, rows, rows) << Eigen::MatrixXd::Identity(rows, rows);
    return constraintMatrix;
}

Eigen::VectorXd MadMaxLpSolver::CreateConstraintVector() {
    int rows = m_statement->timeRate.rows();
    int cols = m_statement->timeRate.cols();
    Eigen::VectorXd constraintVector(cols + 1 + 1 + rows);
    constraintVector << Eigen::VectorXd::Zero(cols), m_statement->maxRisk, m_statement->maxSize, m_statement->maxWeight;
    return constraintVector;
}

Eigen::VectorXd MadMaxLpSolver::CreateObjectiveVector() {
    int rows = m_statement->timeRate.rows();
    int cols = m_statement->timeRate.cols();
    Eigen::VectorXd objectiveVector(rows + cols * 2 + 1 + rows);
    objectiveVector << -m_statement->meanRate, Eigen::VectorXd::Zero(cols * 2 + 1 + rows); 
    return objectiveVector;
}

MadSolution & MadMaxLpSolver::Current() {
    Eigen::VectorXd point = m_solver->GetPoint();
    m_solution.accuracy = 0;
    m_solution.iterationAmount = m_statement->iterationLimit - m_solver->GetIterations();
    m_solution.nodeAmount = 1;
    m_solution.objectiveValue = -m_solver->GetValue();
    m_solution.problemType = ProblemType::MAD_MAX_LP;
    m_solution.weights = point.block(0, 0, m_statement->timeRate.rows(), 1);
    m_solution.returnValue = m_solution.weights.transpose() * m_statement->meanRate;
    m_solution.riskValue = m_solution.objectiveValue;
    m_solution.time = m_solver->GetDuration();
    m_solution.status = m_solver->GetStatus();
    return m_solution;
}