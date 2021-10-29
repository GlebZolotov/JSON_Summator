#pragma once

/// **************************************************
/// Статусы метода решения задачи ЛП
/// **************************************************
enum class LinearStatus {
    /// **************************************************
    /// Решение линейной задачи найдено и существует
    /// **************************************************
    FOUND,
    /// **************************************************
    /// Решение линейной задачи не существует
    /// **************************************************
    NOT_EXIST,
    /// **************************************************
    /// Достигнут лимит по итерациям
    /// **************************************************
    ITERATION_LIMIT,
    /// **************************************************
    /// Достигнут лимит по целевой функции
    /// **************************************************
    OBJECTIVE_LIMIT,
    /// **************************************************
    /// Задача решается - решение не найдено
    /// **************************************************
    NOT_FOUND,
    /// **************************************************
    /// Решение задачи прервано
    /// **************************************************
    INTERRUPTED
};

inline std::ostream &operator<<(std::ostream &lhs, LinearStatus s)
{
    switch (s)
    {
    case LinearStatus::FOUND:
        lhs << "FOUND";
        break;
    case LinearStatus::NOT_EXIST:
        lhs << "NOT_EXIST";
        break;
    case LinearStatus::ITERATION_LIMIT:
        lhs << "ITERATION_LIMIT";
        break;
    case LinearStatus::OBJECTIVE_LIMIT:
        lhs << "OBJECTIVE_LIMIT";
        break;
    case LinearStatus::NOT_FOUND:
        lhs << "NOT_FOUND";
        break;
    case LinearStatus::INTERRUPTED:
        lhs << "INTERRUPTED";
        break;
    default:
        lhs << "UNDEFINED_STATUS";
    }
    return lhs;
}