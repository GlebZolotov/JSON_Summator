#pragma once

enum class SolutionStatus {
    // Решение не начато
    NOT_STARTED,
    // Решается
    STARTED,
    // Найдено оптимальное значение
    OPTIMAL_FOUND,
    // Некорректная задача
    INCORRECT_PROBLEM, // INFEASIBLE
    // Неограниченное решение
    UNBOUNDED_PROBLEM,
    // Достигнуто максимальное число итераций
    ITERATION_LIMIT_REACHED,
    // Пройдено максимальное число узлов
    NODE_LIMIT_REACHED,
    // Достигнут лимит по времени расчета
    TIME_LIMIT_REACHED,
    // Расчет прерван
    INTERRUPTED,
    // Вычислительная ошибка
    NUMERICAL_ERROR,
    // Найдено субоптимальное решение
    SUBOPTIMAL_FOUND,
    // Достигнут лимит по ограничению пользователя
    USER_OBJ_LIMIT_REACHED,
    // Задача не поддерживается
    NOT_SUPPORTED
};
