#pragma once

#include <eigen3/Eigen/Dense>

// Формулировка задачи минимизации / максимизации
struct MadStatement {
    // Матрица доходности (n_asset x Time)
    Eigen::MatrixXd timeRate;
    // Средний вектор доходности (n_asset)
    Eigen::VectorXd meanRate;
    // Максимальный вес архива в портфеле (n_asset, 0 < maxWeight <= 1)
    Eigen::VectorXd maxWeight;
    // Минимальная доходность
    double minReturn;
    // Максимальный риск
    double maxRisk;
    // Число единиц в одном пакете (n_asset, >0)
    Eigen::VectorXi lotSize;
    // Цена закрытия (n_asset, >0)
    Eigen::VectorXd close;
    // Объем капитала (>0)
    double capital;
    // Минимальный относительный объем портфеля (0<=minSize<=maxSize)
    double minSize;
    // Максимальный относительный объем портфеля (0<=minSize<=maxSize)
    double maxSize = 1;
    // Ограничение на целевую функцию задачи
    double objectiveUserLimit;
    // Ограничение на число итераций
    int iterationLimit;
    // Ограничение на число узлов
    int nodeLimit;
    // Ограничение на время расчёта
    double timeLimit;
};
