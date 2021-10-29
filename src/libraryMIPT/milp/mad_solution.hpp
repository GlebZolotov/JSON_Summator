#pragma once

#include <Eigen/Dense>
#include "../solution_status.hpp"
#include "../problem_type.hpp"
#include <string>
#include <vector>

/// *************************************
/// Решение задачи MAD
/// *************************************
struct MadSolution {
    /// *************************************
    /// Стоимость портфеля
    /// *************************************
    double totalCost;

    /// *************************************
    /// Значение целевой функции
    /// *************************************
    double objectiveValue;

    /// *************************************
    /// количество активов
    /// *************************************
    Eigen::VectorXi numLots;

    /// *************************************
    /// Веса активов
    /// *************************************
    Eigen::VectorXd weights;

    /// *************************************
    /// Тип решаемой проблемы
    /// *************************************
    ProblemType problemType;

    /// *************************************
    /// Результат решения
    /// *************************************
    SolutionStatus status;

    /// *************************************
    /// точность решения
    /// *************************************
    double accuracy;

    /// *************************************
    /// число итераций
    /// *************************************
    unsigned int iterationAmount;

    /// *************************************
    /// число узлов
    /// *************************************
    unsigned int nodeAmount;

    /// *************************************
    /// продолжительность решения (в секундах)
    /// *************************************
    double time;

    /// *************************************
    /// риск (значение)
    /// *************************************
    double riskValue;

    /// *************************************
    /// доходность
    /// *************************************
    double returnValue;
};