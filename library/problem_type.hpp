#pragma once

// Список задач
enum class ProblemType {
    // Задача MILD MAD минимизации
    MAD_MIN,
    // Задача MILD MAD максимизации
    MAD_MAX,
    // Задача LP MAD максимизации
    MAD_MAX_LP,
    // Задача LP MAD минимизации
    MAD_MIN_LP
};
