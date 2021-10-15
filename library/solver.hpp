#pragma once

// Базовый класс решателей
template <class T>
class Solver {
    public:
        virtual ~Solver() = 0;
        // Решить задачу оптимизации
        virtual T & Solve() = 0;
        // Прервать решение задачи оптимизации
        virtual void Interrupt() = 0;
        // Получить текущее решение задачи оптимизации
        virtual T & Current() = 0;
};

template <class T>
inline Solver<T>::~Solver() { }