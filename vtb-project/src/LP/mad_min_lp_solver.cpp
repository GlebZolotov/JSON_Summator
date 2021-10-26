#include "mad_min_lp_solver.hpp"

#include <iostream>
#include <Eigen/Dense>
#include <vector>
#include <time.h>

MadMinLpSolver::MadMinLpSolver(MadStatement * statement, LpSolver * linearSolver) : m_statement(statement) 
{
    m_solver = linearSolver;
}

MadMinLpSolver::~MadMinLpSolver() {

}

MadSolution & MadMinLpSolver::Solve() {
    Eigen::MatrixXd constraintMatrix = CreateConstraintMatrix();
    Eigen::VectorXd constraintVector = CreateConstraintVector();
    Eigen::VectorXd objectiveVector = CreateObjectiveVector();
    int iterations = m_statement->iterationLimit;
    m_solver->Solve(objectiveVector, constraintMatrix, constraintVector, iterations);
    return Current();
}

void MadMinLpSolver::Interrupt() {
    m_solver->Interrupt();
}

Eigen::MatrixXd MadMinLpSolver::CreateConstraintMatrix() {
    int rows = m_statement->timeRate.rows();
    int cols = m_statement->timeRate.cols();
    Eigen::MatrixXd constraintMatrix(cols + 1 + 1 + rows, rows + cols * 2 + 1 + rows);
    std::cout << m_statement->meanRate.rows() << " x " << m_statement->meanRate.cols();
    Eigen::MatrixXd rateMatrix = m_statement->timeRate.colwise() - m_statement->meanRate;
    constraintMatrix.block(0,0,cols, rows + cols * 2) << rateMatrix.transpose(), -Eigen::MatrixXd::Identity(cols, cols), Eigen::MatrixXd::Identity(cols, cols);
    constraintMatrix.block(cols + 0, 0, 1, rows + cols * 2) << m_statement->meanRate.transpose(), Eigen::RowVectorXd::Zero(cols + cols);
    constraintMatrix.block(cols + 1, 0, 1, rows + cols * 2) << Eigen::RowVectorXd::Constant(rows, 1.0), Eigen::RowVectorXd::Zero(cols + cols);
    constraintMatrix.block(cols + 2, 0, rows, rows + cols * 2) << Eigen::MatrixXd::Identity(rows,rows), Eigen::MatrixXd::Zero(rows, cols * 2);

    constraintMatrix.block(0, rows + cols * 2, cols, 1 + rows) << Eigen::MatrixXd::Zero(cols, 1 + rows);
    constraintMatrix.block(cols + 0, rows + cols * 2, 1, 1 + rows) << -1.0, Eigen::RowVectorXd::Zero(rows);
    constraintMatrix.block(cols + 1, rows + cols * 2, 1, 1 + rows) << Eigen::RowVectorXd::Zero(1 + rows);
    constraintMatrix.block(cols + 2, rows + cols * 2, rows, 1) << Eigen::VectorXd::Zero(rows, 1);
    constraintMatrix.block(cols + 2, rows + cols * 2 + 1, rows, rows) << Eigen::MatrixXd::Identity(rows, rows);
    return constraintMatrix;
}

Eigen::VectorXd MadMinLpSolver::CreateConstraintVector() {
    int rows = m_statement->timeRate.rows();
    int cols = m_statement->timeRate.cols();
    Eigen::VectorXd constraintVector(cols + 1 + 1 + rows);
    std::cout << "max weight: " << m_statement->maxWeight.rows() << " x " << m_statement->maxWeight.cols() << std::endl;
    constraintVector << Eigen::VectorXd::Zero(cols), m_statement->minReturn, m_statement->maxSize, m_statement->maxWeight;
    return constraintVector;
}

Eigen::VectorXd MadMinLpSolver::CreateObjectiveVector() {
    int rows = m_statement->timeRate.rows();
    int cols = m_statement->timeRate.cols();
    Eigen::VectorXd objectiveVector(rows + cols * 2 + 1 + rows);
    objectiveVector << Eigen::VectorXd::Zero(rows + cols), Eigen::VectorXd::Constant(cols, 1.0), Eigen::VectorXd::Zero(1 + rows); 
    return objectiveVector;
}


MadSolution & MadMinLpSolver::Current() {
    Eigen::VectorXd point = m_solver->GetPoint();
    m_solution.accuracy = 0;
    m_solution.iterationAmount = m_statement->iterationLimit - m_solver->GetIterations();
    m_solution.nodeAmount = 1;
    m_solution.objectiveValue = m_solver->GetValue();
    m_solution.problemType = ProblemType::MAD_MIN_LP;
    m_solution.weights = point.block(0, 0, m_statement->timeRate.rows(), 1);
    m_solution.returnValue = m_solution.weights.transpose() * m_statement->meanRate;
    m_solution.riskValue = m_solution.objectiveValue;
    m_solution.time = m_solver->GetDuration();
    m_solution.status = m_solver->GetStatus();
    return m_solution;
}