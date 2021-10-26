#pragma once

enum class SolutionStatus {
    /// Решение не начато
    NOT_STARTED,
    /// Решается
    STARTED,
    /// Найдено оптимальное значение
    OPTIMAL_FOUND,
    /// Некорректная задача
    INCORRECT_PROBLEM, // INFEASIBLE
    /// Неограниеченно решение
    UNBOUNDED_PROBLEM,
    /// Достигнуто максимальное число итераций
    ITERATION_LIMIT_REACHED,
    /// Пройдено максимальное число узлов
    NODE_LIMIT_REACHED,
    /// Достигнурт лимит по времени расчета
    TIME_LIMIT_REACHED,
    /// Расчет прерван
    INTERRUPTED,
    /// Вычислительная ошибка
    NUMERICAL_ERROR,
    /// Найдено субоптимальное решение
    SUBOPTIMAL_FOUND,
    /// Достигнут лимит по ограничению пользователя
    USER_OBJ_LIMIT_REACHED,
    /// Задача не поддерживается
    NOT_SUPPORTED
};

inline std::ostream &operator<<(std::ostream &lhs, SolutionStatus s)
{
    switch (s)
    {
    case SolutionStatus::NOT_STARTED:
        lhs << "NOT_STARTED";
        break;
    case SolutionStatus::STARTED:
        lhs << "STARTED";
        break;
    case SolutionStatus::OPTIMAL_FOUND:
        lhs << "OPTIMAL_FOUND";
        break;
    case SolutionStatus::INCORRECT_PROBLEM:
        lhs << "INCORRECT_PROBLEM";
        break;
    case SolutionStatus::UNBOUNDED_PROBLEM:
        lhs << "UNBOUNDED_PROBLEM";
        break;
    case SolutionStatus::ITERATION_LIMIT_REACHED:
        lhs << "ITERATION_LIMIT_REACHED";
        break;
    case SolutionStatus::NODE_LIMIT_REACHED:
        lhs << "NODE_LIMIT_REACHED";
        break;
    case SolutionStatus::TIME_LIMIT_REACHED:
        lhs << "TIME_LIMIT_REACHED";
        break;
    case SolutionStatus::INTERRUPTED:
        lhs << "INTERRUPTED";
        break;
    case SolutionStatus::NUMERICAL_ERROR:
        lhs << "NUMERICAL_ERROR";
        break;
    case SolutionStatus::SUBOPTIMAL_FOUND:
        lhs << "SUBOPTIMAL_FOUND";
        break;
    case SolutionStatus::USER_OBJ_LIMIT_REACHED:
        lhs << "USER_OBJ_LIMIT_REACHED";
        break;
    case SolutionStatus::NOT_SUPPORTED:
        lhs << "NOT_SUPPORTED";
        break;
    default:
        lhs << "UNDEFINED_STATUS";
    }
    return lhs;
}
