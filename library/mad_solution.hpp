#pragma once

#include <eigen3/Eigen/Dense>
#include "solution_status.hpp"
#include "problem_type.hpp"
#include <string>
#include <vector>

// Решение задачи MAD
struct MadSolution {
    // Стоимость портфеля
    double totalCost;
    // Значение целевой функции
    double objectiveValue;
    // Количество активов
    Eigen::VectorXi numLots;
    // Веса активов
    Eigen::VectorXd weights;
    // Тип решаемой проблемы
    ProblemType problemType;
    // Результат решения
    SolutionStatus status;
    // Точность решения
    double accuracy;
    // Число итераций
    unsigned int iterationAmount;
    // Число узлов
    unsigned int nodeAmount;
    // Продолжительность решения в секундах
    double time;
    // Риск (значение)
    double riskValue;
    // Доходность
    double returnValue;  
};
