#pragma once

#include <Eigen/Dense>
#include <vector>
#include "linear_status.hpp"

class SimplexMethod {
    private:
        double m_zeroThreshold;

    public:
        /// **************************************************
        /// Конструктор солвера
        /// @param zeroThreshold - какую величину считаем 0
        /// **************************************************
        SimplexMethod(double zeroThreshold);

        /// **************************************************
        /// Деструктор солвера
        /// **************************************************
        ~SimplexMethod();

        /// **************************************************
        /// Настроить ограничения задачи, для корректного применения симплекс метода
        /// @param constraintMatrix - матрица ограничений задачи
        /// @param constraintVector - вектор ограничений задачи
        /// **************************************************
        void Adjust(Eigen::MatrixXd & constraintMatrix, Eigen::VectorXd &constraintVector);

        /// **************************************************
        /// Найти базисную (крайнюю) точку
        /// @param constraintMatrix - матрица ограничений задачи
        /// @param constraintVector - вектор ограничений задачи
        /// @param basisColumns - список базисных переменных
        /// @returns
        ///     вектор стандартный индексов базисных переменных
        /// @remarks
        ///     это 1 фаза симплекс метода. При этом матрица ограничений constraintMatrix и вектор ограничений constraintVector изменяются согласно выбранному базису. 
        ///     Т.е. вектор в точности соответствует базисной точки, а матрица ограничений включает единичную матрицу для базисных переменных
        ///     список базисных переменных заполняется после решения задачи
        /// **************************************************
        void FindBasis(Eigen::MatrixXd & constraintMatrix, Eigen::VectorXd &constraintVector, std::vector<int> & basisColumns);

        /// **************************************************
        /// Решить задачу ЛП двухфазным симплекс-методом
        /// @param objectiveVector - вектор, определяющий целевую функцию задачи
        /// @param constraintMatrix - матрица ограничений задачи
        /// @param constraintVector - вектор ограничений задачи
        /// @param basisColumns - список базисных переменных
        /// @returns
        ///     результирующая симплекс таблица
        /// **************************************************
        Eigen::MatrixXd SolveTwoPhase(Eigen::VectorXd & objectiveVector, Eigen::MatrixXd & constraintMatrix, Eigen::VectorXd &constraintVector, 
            std::vector<int> & basisColumns);

        /// **************************************************
        /// Решить задачу ЛП с известной начальной точкой
        /// @param objectiveVector - вектор, определяющий целевую функцию задачи
        /// @param constraintMatrix - матрица ограничений задачи
        /// @param constraintVector - вектор ограничений задачи
        /// @param basisColumns - список базисных переменных
        /// @returns
        ///     результирующая симплекс таблица
        /// **************************************************
        Eigen::MatrixXd Solve(Eigen::VectorXd & objectiveVector, Eigen::MatrixXd & constraintMatrix, Eigen::VectorXd &constraintVector, 
            std::vector<int> & basisColumns);

        /// **************************************************
        /// Проверить есть ли решение
        /// @param simplexTable - симплекс таблица
        /// @returns
        ///     > 0 - решение найдено
        ///     = 0 - решение не найдено
        ///     < 0 - решения нет
        /// **************************************************
        LinearStatus Check(Eigen::MatrixXd & simplexTable);


        /// **************************************************
        /// Сделать шаг
        /// @param simplexTable - симплекс таблица
        /// @param basisColumns - список базисных переменных
        /// @returns
        ///     нет
        /// @remarks
        ///     вводит новую базисную переменную обновляя симплекс-таблицу и список базисных переменных
        /// **************************************************
        void DoStep(Eigen::MatrixXd & simplexTable, std::vector<int> & basisColumns);


        /// **************************************************
        /// Решить задачу ЛП
        /// @param simplexTable - симплекс таблица
        /// @param basisColumns - список базисных переменных
        /// @param iterationAmount - ссылка на максимальное число итераций.
        /// @param stop флаг остановки рассчета
        /// @returns LinearStatus
        /// @remarks
        ///     на каждой итерации значение iterationAmount уменьшается на 1
        ///     решение задачи определено в Eigen::MatrixXd & simplexMatrix, std::vector<int> & basisColumns
        /// **************************************************
        LinearStatus Solve(Eigen::MatrixXd & simplexMatrix, std::vector<int> & basisColumns, int & iterationAmount, bool & stop);

        /// **************************************************
        /// Решить задачу ЛП
        /// @param simplexTable - симплекс таблица
        /// @param basisColumns - список базисных переменных
        /// @param iterationAmount - ссылка на максимальное число итераций.
        /// @param stop - флаг остановки рассчета
        /// @param minObjValue - требование пользователя на остановку рассчета
        /// @returns LinearStatus
        /// @remarks
        ///     на каждой итерации значение iterationAmount уменьшается на 1
        ///     решение задачи определено в Eigen::MatrixXd & simplexMatrix, std::vector<int> & basisColumns
        /// **************************************************
        LinearStatus Solve(Eigen::MatrixXd & simplexMatrix, std::vector<int> & basisColumns, 
            int & iterationAmount, bool & stop, double minObjValue);

        /// **************************************************
        /// Построить симплекс таблицу
        /// @param objectiveVector - вектор, определяющий целевую функцию задачи
        /// @param constraintMatrix - матрица ограничений задачи
        /// @param constraintVector - вектор ограничений задачи
        /// @param basisColumns - список базисных переменных
        /// @param inverse - нужно ли рассчитывать обратную матрицу?
        /// @returns
        ///     результирующая симплекс таблица
        /// **************************************************
        Eigen::MatrixXd BuildSimplexTable(Eigen::VectorXd & objectiveVector, Eigen::MatrixXd & constraintMatrix, Eigen::VectorXd &constraintVector, 
            std::vector<int> & basisColumns, bool inverse);

        /// **************************************************
        /// Извлечь базисную точку из симплекс таблицы
        /// @param simplexTable симплекс-таблица
        /// @param basisColumns список базисных столбцов
        /// **************************************************
        Eigen::VectorXd ExtractPoint(Eigen::MatrixXd & simplexTable, std::vector<int> & basisColumns);

        /// **************************************************
        /// Извлечь значение целевой функции из симплекс-таблицы
        /// @param simplexTable симплекс-таблица
        /// **************************************************
        double ExtractValue(Eigen::MatrixXd & simplexTable);
};