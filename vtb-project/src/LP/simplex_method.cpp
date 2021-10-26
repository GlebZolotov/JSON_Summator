#include <iostream>
#include "simplex_method.hpp"

SimplexMethod::SimplexMethod(double zeroThreshold) {
    m_zeroThreshold = zeroThreshold;
}

SimplexMethod::~SimplexMethod() {

}

void SimplexMethod::Adjust(Eigen::MatrixXd & constraintMatrix, Eigen::VectorXd &constraintVector) {
    for (int i = 0; i< constraintVector.rows(); i++)
    {
        if (constraintVector[i] > -m_zeroThreshold) {
            continue;
        }
        constraintVector[i] = -constraintVector[i];
        constraintMatrix.row(i) *= (-1); 
    }
}


void SimplexMethod::FindBasis(Eigen::MatrixXd & constraintMatrix, Eigen::VectorXd &constraintVector, std::vector<int> & basisColumns) {
    Eigen::VectorXd objectiveVector(constraintMatrix.cols() + constraintMatrix.rows());
    objectiveVector << Eigen::VectorXd::Zero(constraintMatrix.cols()), Eigen::VectorXd::Constant(constraintMatrix.rows(), 1);
    for (int i = 0; i < constraintMatrix.rows(); i++)
    {
        basisColumns.push_back(i + constraintMatrix.cols());
    }
    Eigen::MatrixXd extendedMatrix(constraintMatrix.rows(), constraintMatrix.cols() + constraintMatrix.rows());
    extendedMatrix.block(0, 0, constraintMatrix.rows(), constraintMatrix.cols()) << constraintMatrix;
    extendedMatrix.block(0, constraintMatrix.cols(), constraintMatrix.rows(), constraintMatrix.rows()) << 
        Eigen::MatrixXd::Identity(constraintMatrix.rows(), constraintMatrix.rows());
    Eigen::MatrixXd simplexTable = BuildSimplexTable(objectiveVector, extendedMatrix, constraintVector, basisColumns, false);
    int iterationAmount = -1;
    bool stop = false;
    Solve(simplexTable, basisColumns, iterationAmount, stop);
    for (unsigned int i = 0; i < basisColumns.size(); i++) {
        int colIndex = basisColumns[i];
        if (colIndex < constraintMatrix.rows()) {
            continue;
        }
        Eigen::MatrixXd::Index rowIndex;
        simplexTable.col(colIndex + 1).maxCoeff(&rowIndex);
        Eigen::MatrixXd::Index newColIndex;
        simplexTable.block(rowIndex, 1, 1, constraintMatrix.cols()).row(0).cwiseAbs().maxCoeff(&newColIndex);
        double val = simplexTable(rowIndex, newColIndex + 1);
        auto row = simplexTable.row(rowIndex);
        row /= val;
        basisColumns[i] = newColIndex;
        for (int j = 0; j < constraintMatrix.rows(); j++) {
            if (j == rowIndex) {
                continue;
            }
            double coeff = simplexTable(j, newColIndex + 1);
            simplexTable.block(j, 0, 1, simplexTable.cols()) -= row * coeff;
        }
    }
    constraintVector << simplexTable.block(1, 0, constraintVector.rows(), 1);
    constraintMatrix << simplexTable.block(1, 1, constraintMatrix.rows(), constraintMatrix.cols());
}

Eigen::MatrixXd SimplexMethod::SolveTwoPhase(Eigen::VectorXd & objectiveVector, Eigen::MatrixXd & constraintMatrix, Eigen::VectorXd &constraintVector, 
            std::vector<int> & basisColumns) {
    Adjust(constraintMatrix, constraintVector);
    FindBasis(constraintMatrix, constraintVector, basisColumns);
    return Solve(objectiveVector, constraintMatrix, constraintVector, basisColumns);
}

Eigen::MatrixXd SimplexMethod::Solve(Eigen::VectorXd & objectiveVector, Eigen::MatrixXd & constraintMatrix, Eigen::VectorXd &constraintVector, 
    std::vector<int> & basisColumns) {
        Eigen::MatrixXd simplexTable = BuildSimplexTable(objectiveVector, constraintMatrix, constraintVector, basisColumns, true);
        int maxIterationAmount = -1;
        bool stop = false;
        Solve(simplexTable, basisColumns, maxIterationAmount, stop);
        return simplexTable;
    
}

void SimplexMethod::DoStep(Eigen::MatrixXd & simplexTable, std::vector<int> & basisColumns) {
    // Ищем индекс переменной которую будем вводить - столбец с максимальным положительным элементом.
    Eigen::MatrixXd::Index index;
    Eigen::MatrixXd::Index supIndex;
    simplexTable.block(0, 1, 1, simplexTable.cols() - 1).maxCoeff(&supIndex, &index);
    //simplexTable.row(0).tail(simplexTable.cols() - 1).maxCoeff(&index);

    // выделяем столбец по которому смотрим
    auto changeColumn = simplexTable.block(1, index + 1, simplexTable.rows() - 1, 1);

    // выделяем базовый столбей
    auto basisColumn = simplexTable.block(1, 0, simplexTable.rows() - 1, 1);
    double currentValue = 0;
    int renewIndex = -1;
    // ищем переменную на замену
    for (int i = 0; i < simplexTable.rows() - 1; i++)
    {
        double basisValue = basisColumn(i, 0);
        double changeValue = changeColumn(i, 0);
        if (changeColumn(i, 0) < m_zeroThreshold) {
            continue;
        }
        
        if (renewIndex == -1) {
            renewIndex = i;
            currentValue = basisValue/changeValue;
            continue;
        }
        
        if (currentValue > basisValue/changeValue) {
            renewIndex = i;
            currentValue = basisValue/changeValue;
        }
    }

    // вносим новую базисную переменную
    basisColumns[renewIndex] = index;
    simplexTable.row(renewIndex + 1) << simplexTable.row(renewIndex + 1) / changeColumn(renewIndex, 0);
    for (int i = 0; i < simplexTable.rows(); i++)
    {
        if (i == renewIndex + 1) {
            continue;
        }
        simplexTable.row(i) -= simplexTable.row(renewIndex + 1) * simplexTable(i, index + 1);
    }
}

LinearStatus SimplexMethod::Check(Eigen::MatrixXd & simplexTable) {
    auto negativeElements = (simplexTable.row(0).tail(simplexTable.cols() - 1).array() <= m_zeroThreshold);
    // Проверяем найдена ли точка минимума (все элементы 0-ой строки матрицы не положительны)
    if (negativeElements.all())
    {
        return LinearStatus::FOUND;
    }
    // Проверяем на существование решения задачи (а вдруг нет)...
    auto positiveCheck = (simplexTable.block(1,1, simplexTable.rows() - 1, simplexTable.cols() - 1).array() > m_zeroThreshold)
        .cast<int>()
        .colwise()
        .maxCoeff();
    auto existanseCheck = ((positiveCheck + negativeElements.cast<int>()).array() == 0);
    if (existanseCheck.any()) {
        return LinearStatus::NOT_EXIST;
    }
    return LinearStatus::NOT_FOUND;
}

LinearStatus SimplexMethod::Solve(Eigen::MatrixXd & simplexMatrix, std::vector<int> & basisColumns, 
    int & iterationAmount, bool & stop) {
    while (iterationAmount != 0 && !stop)
    {
        iterationAmount--;
        LinearStatus checkResult = Check(simplexMatrix);
        if (checkResult != LinearStatus::NOT_FOUND) {
            return checkResult;
        }
        DoStep(simplexMatrix, basisColumns);
    }
    return LinearStatus::ITERATION_LIMIT;
}

LinearStatus SimplexMethod::Solve(Eigen::MatrixXd & simplexMatrix, std::vector<int> & basisColumns, 
    int & iterationAmount, bool & stop, double minObjValue) {
    while (iterationAmount != 0 && !stop)
    {
        iterationAmount--;
        LinearStatus checkResult = Check(simplexMatrix);
        if (checkResult != LinearStatus::NOT_FOUND) {
            return checkResult;
        }
        if (ExtractValue(simplexMatrix) < minObjValue) {
            return LinearStatus::OBJECTIVE_LIMIT;
        }
        DoStep(simplexMatrix, basisColumns);
    }
    return LinearStatus::ITERATION_LIMIT;
}

Eigen::MatrixXd SimplexMethod::BuildSimplexTable(
    Eigen::VectorXd & objectiveVector, 
    Eigen::MatrixXd & constraintMatrix, 
    Eigen::VectorXd & constraintVector,  
    std::vector<int> & basisColumns,
    bool inverse) {

    if (inverse)
    {
        Eigen::MatrixXd basisMatrix = constraintMatrix(Eigen::all,  basisColumns);
        Eigen::MatrixXd inverseMatrix = basisMatrix.inverse();
        Eigen::MatrixXd reducedMatrix = inverseMatrix * constraintMatrix;
        Eigen::VectorXd reducedVector = inverseMatrix * constraintVector;
        
        Eigen::MatrixXd simplexMatrix(reducedMatrix.rows() + 1, reducedMatrix.cols() + 1);
        simplexMatrix.block(0, 0, 1, 1) << objectiveVector(Eigen::all, basisColumns).transpose() * reducedVector;
        simplexMatrix.col(0).tail(reducedMatrix.rows()) << reducedVector;
        simplexMatrix.block(1,1, reducedMatrix.rows(), reducedMatrix.cols()) << reducedMatrix;
        simplexMatrix.block(0, 1, 1, reducedMatrix.cols()) << objectiveVector(Eigen::all, basisColumns).transpose() * reducedMatrix - objectiveVector.transpose();
        return simplexMatrix;
    }
    else
    {
        Eigen::MatrixXd simplexMatrix(constraintMatrix.rows() + 1, constraintMatrix.cols() + 1);
        simplexMatrix.block(0, 0, 1, 1) = (objectiveVector.transpose())(Eigen::all, basisColumns) * constraintVector;
        simplexMatrix.block(1, 0, constraintMatrix.rows(), 1) << constraintVector;
        simplexMatrix.block(1, 1, constraintMatrix.rows(), constraintMatrix.cols()) << constraintMatrix;
        simplexMatrix.block(0, 1, 1, constraintMatrix.cols()) << (objectiveVector.transpose())(Eigen::all, basisColumns) * constraintMatrix - objectiveVector.transpose();
        return simplexMatrix;
    }
}

Eigen::VectorXd SimplexMethod::ExtractPoint(Eigen::MatrixXd & simplexTable, std::vector<int> & basisColumns) {
    Eigen::VectorXd result = Eigen::VectorXd::Zero(simplexTable.cols() - 1);
    for (unsigned int i = 0; i < basisColumns.size(); i++) {
        int coordinate = basisColumns[i];
        result[coordinate] = simplexTable(i + 1, 0);
    }
    return result;
}

double SimplexMethod::ExtractValue(Eigen::MatrixXd & simplexTable) {
    return simplexTable(0, 0);
}