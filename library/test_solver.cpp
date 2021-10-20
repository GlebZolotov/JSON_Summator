#include "test_solver.hpp"

TestSolver::TestSolver(MadStatement & t) {
    task.timeRate = t.timeRate;
    task.meanRate = t.meanRate;
    task.maxWeight = t.maxWeight;
    task.minReturn = t.minReturn;
    task.maxRisk = t.maxRisk;
    task.lotSize = t.lotSize;
    task.close = t.close;
    task.capital = t.capital;
    task.maxSize = t.maxSize;
    task.minSize = t.minSize;
    task.objectiveUserLimit = t.objectiveUserLimit;
    task.iterationLimit = t.iterationLimit;
    task.nodeLimit = t.nodeLimit;
    task.timeLimit = t.timeLimit;
};

MadSolution & TestSolver::Current() {
    return res;
};

void TestSolver::Interrupt() {
    res.status = SolutionStatus::INTERRUPTED;
};

MadSolution & TestSolver::Solve() {
    std::random_device rd; 
    std::mt19937 mersenne(rd());
    res.accuracy = mersenne()%10000 / 100000.;
    res.totalCost = mersenne()%10000 / 100.;
    res.iterationAmount = mersenne()%10000;
    res.nodeAmount = mersenne()%10000;
    res.numLots.resize(task.meanRate.rows());
    res.weights.resize(task.meanRate.rows());
    for (int i = 0; i < task.meanRate.rows(); i++) {
        res.numLots[i] = mersenne()%10000;
        res.weights[i] = (double)(mersenne()%10000) / 100000.;
    }
    res.objectiveValue = mersenne()%10000 / 100000.;
    res.problemType = static_cast<ProblemType>(mersenne()%4);
    res.returnValue = 0;
    res.riskValue = mersenne()%10000 / 100000.;
    res.status = SolutionStatus::STARTED;
    res.time = mersenne()%10000 / 100000.;
    for (int i = 0; i < 5 + mersenne()%5; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        if (res.status == SolutionStatus::INTERRUPTED) return res;
        res.returnValue += 1.;
    }
    res.status = static_cast<SolutionStatus>(2 + mersenne()%11);
    return res;
}