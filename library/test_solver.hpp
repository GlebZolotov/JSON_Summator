#pragma once

#include "solver.hpp"
#include "mad_solution.hpp"
#include "mad_statement.hpp"
#include <chrono>
#include <thread>
#include <random>

class TestSolver : public Solver<MadSolution> {
    public:
        TestSolver(MadStatement & t);
        MadSolution & Solve();
        MadSolution & Current();
        void Interrupt();
    private:
        MadStatement task;
        MadSolution res;
};
