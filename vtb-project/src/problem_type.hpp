#pragma once

/// Список задач
enum class ProblemType {
    /// Задача MILP MAD минимизации
    MAD_MIN, 
    /// Задача MILP MAD максимизации
    MAD_MAX,
    /// Задача LP MAD максимизации
    MAD_MAX_LP,
    /// Задача LP MAD минимизации
    MAD_MIN_LP,
    /// Задача неопределена
    UNDEFINED,
};