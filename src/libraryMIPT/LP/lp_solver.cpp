#include "lp_solver.hpp"

#include <Eigen/Dense>
#include <vector>
#include <time.h>

LpSolver::LpSolver(SimplexMethod * simplexMethod) {
    m_simplexMethod = simplexMethod;
    m_statusMap = {
        {LinearStatus::FOUND, SolutionStatus::OPTIMAL_FOUND},
        {LinearStatus::ITERATION_LIMIT, SolutionStatus::ITERATION_LIMIT_REACHED},
        {LinearStatus::NOT_EXIST, SolutionStatus::UNBOUNDED_PROBLEM},
        {LinearStatus::NOT_FOUND, SolutionStatus::UNBOUNDED_PROBLEM},
        {LinearStatus::OBJECTIVE_LIMIT, SolutionStatus::USER_OBJ_LIMIT_REACHED},
        {LinearStatus::INTERRUPTED, SolutionStatus::INTERRUPTED}
    };
}

LpSolver::~LpSolver() {

}

void LpSolver::Solve(Eigen::VectorXd & objectiveVector, Eigen::MatrixXd & constraintMatrix,  Eigen::VectorXd & constraintVector,
        int & iterationLimit) {
    m_status = SolutionStatus::STARTED;
    m_tStart = clock();
    m_tStop = clock();
    m_simplexMethod->Adjust(constraintMatrix, constraintVector);
    m_simplexMethod->FindBasis(constraintMatrix, constraintVector, m_basisColumns);
    m_simplexTable = m_simplexMethod->BuildSimplexTable(objectiveVector, constraintMatrix, constraintVector, m_basisColumns, false);
    LinearStatus status = m_simplexMethod->Solve(m_simplexTable, m_basisColumns, iterationLimit, this->m_stop);
    m_status = m_statusMap[status];
    m_stop = true;
    m_tStop = clock();
}

void LpSolver::Interrupt() {
    m_status = SolutionStatus::INTERRUPTED;
    m_tStop = clock();
    m_stop = true;
}

Eigen::VectorXd LpSolver::GetPoint() {
    return m_simplexMethod->ExtractPoint(m_simplexTable, m_basisColumns);
}

double LpSolver::GetValue() {
    return m_simplexMethod->ExtractValue(m_simplexTable);
}

SolutionStatus LpSolver::GetStatus() {
    return m_status;
}

int & LpSolver::GetIterations() {
     return m_iterations;
 }

double LpSolver::GetDuration() {
    if (m_stop) {
        return (double)(m_tStop - m_tStart)/CLOCKS_PER_SEC;
    }
    else {
        return (double)(clock() - m_tStart)/CLOCKS_PER_SEC;
    }
}