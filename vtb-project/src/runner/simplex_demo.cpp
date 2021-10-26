#include <iostream>
#include <Eigen/Dense>
#include "params.hpp"
#include "./milp/mad_statement.hpp"
#include "./LP/simplex_method.hpp"
#include "./LP/lp_solver.hpp"
#include "./LP/mad_min_lp_solver.hpp"
#include <time.h>

bool test0(double threshold) {
    auto tStart = clock();
    std::cout << "\n\x1B[32m-----------------# Test 0 #-----------------\033[0m\t\t" << std::endl;
    SimplexMethod linearSolver(threshold);
    Eigen::MatrixXd constraintM(3, 4);
    constraintM <<  3,-5,1, 0,
                   1, 2,0, 2,
                  -2, 1,0,-1;
    Eigen::VectorXd objective(4);
    objective << 3,1, 0,-1;
    Eigen::VectorXd constraintV(3);
    constraintV << 1,2,1;
    std::vector<int> columns;
    linearSolver.FindBasis(constraintM, constraintV, columns);
    std::cout << "Constraint Matrix: " << std::endl;
    std::cout << constraintM << std::endl;
    std::cout << "Constraint Vector: " << "\n" << constraintV << std::endl;
    std::cout << "columns:";
    for (unsigned int i = 0; i < columns.size(); i++) {
        std::cout << " " << columns[i];
    }
    std::cout << std::endl;
    auto simplex = linearSolver.BuildSimplexTable(objective, constraintM, constraintV, columns, false);
    std::cout<< "Initial Point: " << "\n" << linearSolver.ExtractPoint(simplex, columns) << std::endl;
    std::cout<< "Initial Value: " << linearSolver.ExtractValue(simplex) << std::endl;
    bool stop = false;
    int iterationAmount = -1;
    auto status = linearSolver.Solve(simplex, columns, iterationAmount, stop);
    std::cout << "Solve Status: " << status << std::endl;
    if (status != LinearStatus::FOUND)
        return false;
    std::cout<< "Point: " << "\n" << linearSolver.ExtractPoint(simplex, columns) << std::endl;
    std::cout<< "Value: " << linearSolver.ExtractValue(simplex) << std::endl;
    std::cout<< "Time: \x1B[33m" << (double)(clock() - tStart)/CLOCKS_PER_SEC * 1000 << " ms\033[0m\t\t" << std::endl;
    return true;
}

bool test1(double threshold) {
    auto tStart = clock();
    std::cout << "\n\x1B[32m-----------------# Test 1 #-----------------\033[0m\t\t" << std::endl;
    SimplexMethod linearSolver(threshold);
    Eigen::MatrixXd constraintM(2, 4);
    constraintM <<  1, 2, 5, -1,
                    1, -1,-1, 2;
    Eigen::VectorXd objective(4);
    objective << -2,-1, -3,-1;
    Eigen::VectorXd constraintV(2);
    constraintV << 4, 1;
    std::vector<int> columns;
    linearSolver.FindBasis(constraintM, constraintV, columns);
    std::cout << "Constraint Matrix: " << std::endl;
    std::cout << constraintM << std::endl;
    std::cout << "Constraint Vector: " << "\n" << constraintV << std::endl;
    std::cout << "columns:";
    for (unsigned int i = 0; i < columns.size(); i++) {
        std::cout << " " << columns[i];
    }
    std::cout << std::endl;
    auto simplex = linearSolver.BuildSimplexTable(objective, constraintM, constraintV, columns, false);
    std::cout<< "Initial Point: " << "\n" << linearSolver.ExtractPoint(simplex, columns) << std::endl;
    std::cout<< "Initial Value: " << linearSolver.ExtractValue(simplex) << std::endl;
    bool stop = false;
    int iterationAmount = -1;
    auto status = linearSolver.Solve(simplex, columns, iterationAmount, stop);
    std::cout << "Solve Status: " << status << std::endl;
    if (status != LinearStatus::FOUND)
        return false;
    std::cout<< "Point: " << "\n" << linearSolver.ExtractPoint(simplex, columns) << std::endl;
    std::cout<< "Value: " << linearSolver.ExtractValue(simplex) << std::endl;
    std::cout<< "Time: \x1B[33m" << (double)(clock() - tStart)/CLOCKS_PER_SEC * 1000 << " ms\033[0m\t\t" << std::endl;
    return true;
}

bool test2(double threshold) {
    auto tStart = clock();
    std::cout << "\n\x1B[32m-----------------# Test 2 #-----------------\033[0m\t\t" << std::endl;
    SimplexMethod linearSolver(threshold);
    Eigen::MatrixXd constraintM(5, 7);
    constraintM <<  2, 1, 0, 0, 0, 0, 1,
                    1, 0, 0, 0, -1, 0, 0,
                    0, 1, 0, 0, 0, -1, 0,
                    1, 0, 1, 0, 0, 0, 0,
                    0, 1, 0, 1, 0, 0, 0;
    Eigen::VectorXd objective(7);
    objective << -1, -2, 0, 0, 0, 0, 0;
    Eigen::VectorXd constraintV(5);
    constraintV << 6, 0, 0, 3, 3;
    std::vector<int> columns;
    linearSolver.FindBasis(constraintM, constraintV, columns);
    std::cout << "Constraint Matrix: " << std::endl;
    std::cout << constraintM << std::endl;
    std::cout << "Constraint Vector: " << "\n" << constraintV << std::endl;
    std::cout << "columns:";
    for (unsigned int i = 0; i < columns.size(); i++) {
        std::cout << " " << columns[i];
    }
    std::cout << std::endl;
    auto simplex = linearSolver.BuildSimplexTable(objective, constraintM, constraintV, columns, false);
    std::cout<< "Initial Point: " << "\n" << linearSolver.ExtractPoint(simplex, columns) << std::endl;
    std::cout<< "Initial Value: " << linearSolver.ExtractValue(simplex) << std::endl;
    bool stop = false;
    int iterationAmount = -1;
    auto status = linearSolver.Solve(simplex, columns, iterationAmount, stop);
    std::cout << "Solve Status: " << status << std::endl;
    if (status != LinearStatus::FOUND)
        return false;
    std::cout<< "Point: " << "\n" << linearSolver.ExtractPoint(simplex, columns) << std::endl;
    std::cout<< "Value: " << linearSolver.ExtractValue(simplex) << std::endl;
    if (linearSolver.ExtractValue(simplex) != -7.5)
        return false;
    std::cout<< "Time: \x1B[33m" << (double)(clock() - tStart)/CLOCKS_PER_SEC * 1000 << " ms\033[0m\t\t" << std::endl;
    return true;
}

bool test3(double threshold) {
    auto tStart = clock();
    std::cout << "\n\x1B[32m-----------------# Test 3 #-----------------\033[0m\t\t" << std::endl;
    SimplexMethod linearSolver(threshold);
    Eigen::MatrixXd constraintM(3, 5);
    constraintM <<  0, -1, 1, 1, 0,
                    -5, 1, 1, 0, 0,
                    -8, 1, 2, 0, -1;
    Eigen::VectorXd objective(5);
    objective << 3, -1, -4, 0, 0;
    Eigen::VectorXd constraintV(3);
    constraintV << 1, 2, 3;
    std::vector<int> columns;
    linearSolver.FindBasis(constraintM, constraintV, columns);
    std::cout << "Constraint Matrix: " << std::endl;
    std::cout << constraintM << std::endl;
    std::cout << "Constraint Vector: " << "\n" << constraintV << std::endl;
    std::cout << "columns:";
    for (unsigned int i = 0; i < columns.size(); i++) {
        std::cout << " " << columns[i];
    }
    std::cout << std::endl;
    auto simplex = linearSolver.BuildSimplexTable(objective, constraintM, constraintV, columns, false);
    std::cout<< "Initial Point: " << "\n" << linearSolver.ExtractPoint(simplex, columns) << std::endl;
    std::cout<< "Initial Value: " << linearSolver.ExtractValue(simplex) << std::endl;
    bool stop = false;
    int iterationAmount = -1;
    auto status = linearSolver.Solve(simplex, columns, iterationAmount, stop);
    std::cout << "Solve Status: " << status << std::endl;
    if (status != LinearStatus::FOUND)
        return false;
    std::cout<< "Point: " << "\n" << linearSolver.ExtractPoint(simplex, columns) << std::endl;
    std::cout<< "Value: " << linearSolver.ExtractValue(simplex) << std::endl;
    if (abs(linearSolver.ExtractValue(simplex) - (-16.0)) >= threshold)
        return false;
    std::cout<< "Time: \x1B[33m" << (double)(clock() - tStart)/CLOCKS_PER_SEC * 1000 << " ms\033[0m\t\t" << std::endl;
    return true;
}

bool test4(double threshold) {
    auto tStart = clock();
    std::cout << "\n\x1B[32m-----------------# Test 4 #-----------------\033[0m\t\t" << std::endl;
    SimplexMethod linearSolver(threshold);
    Eigen::MatrixXd constraintM(3, 6);
    constraintM <<  2, -1, 0, -2, 1, 0,
                    3, 2, 1, -3, 0, 0,
                    -1, 3, 0, 4, 0, 1;
    Eigen::VectorXd objective(6);
    objective << -2, -3, 0, 1, 0, 0;
    Eigen::VectorXd constraintV(3);
    constraintV << 16, 18, 24;
    std::vector<int> columns;
    linearSolver.FindBasis(constraintM, constraintV, columns);
    std::cout << "Constraint Matrix: " << std::endl;
    std::cout << constraintM << std::endl;
    std::cout << "Constraint Vector: " << "\n" << constraintV << std::endl;
    std::cout << "columns:";
    for (unsigned int i = 0; i < columns.size(); i++) {
        std::cout << " " << columns[i];
    }
    std::cout << std::endl;
    auto simplex = linearSolver.BuildSimplexTable(objective, constraintM, constraintV, columns, false);
    std::cout<< "Initial Point: " << "\n" << linearSolver.ExtractPoint(simplex, columns) << std::endl;
    std::cout<< "Initial Value: " << linearSolver.ExtractValue(simplex) << std::endl;
    bool stop = false;
    int iterationAmount = -1;
    auto status = linearSolver.Solve(simplex, columns, iterationAmount, stop);
    std::cout << "Solve Status: " << status << std::endl;
    if (status != LinearStatus::FOUND)
        return false;
    std::cout<< "Point: " << "\n" << linearSolver.ExtractPoint(simplex, columns) << std::endl;
    std::cout<< "Value: " << linearSolver.ExtractValue(simplex) << std::endl;
    if (abs(linearSolver.ExtractValue(simplex) - (-282/11)) >= 0)
        return false;
    std::cout<< "Time: \x1B[33m" << (double)(clock() - tStart)/CLOCKS_PER_SEC * 1000 << " ms\033[0m\t\t" << std::endl;
    return true;
}

int main() {
    char successful = 0;
    char failed = 0;

    if (test0(1e-10)) {
        std::cout << "\x1B[32mPASSED\033[0m\t\t" << std::endl;
        ++successful;
    } else {
        std::cout << "\x1B[31mFAILED\033[0m\t\t" << std::endl;
        ++failed;
    }

    if (test1(1e-10)) {
        std::cout << "\x1B[32mPASSED\033[0m\t\t" << std::endl;
        ++successful;
    } else {
        std::cout << "\x1B[31mFAILED\033[0m\t\t" << std::endl;
        ++failed;
    }
    
    if (test2(1e-10)) {
        std::cout << "\x1B[32mPASSED\033[0m\t\t" << std::endl;
        ++successful;
    } else {
        std::cout << "\x1B[31mFAILED\033[0m\t\t" << std::endl;
        ++failed;
    }
    
    // accumulated inaccuracy in objective is 1e-14 
    if (test3(2e-14)) {
        std::cout << "\x1B[32mPASSED\033[0m\t\t" << std::endl;
        ++successful;
    } else {
        std::cout << "\x1B[31mFAILED\033[0m\t\t" << std::endl;
        ++failed;
    }

    // accumulated inaccuracy in objective is 0.4 if thr=1e-20
    if (test4(1e-100)) {
        std::cout << "\x1B[32mPASSED\033[0m\t\t" << std::endl;
        ++successful;
    } else {
        std::cout << "\x1B[31mFAILED\033[0m\t\t" << std::endl;
        ++failed;
    }

    std::cout << "----------------------------------" << std::endl;
    std::cout << "\x1B[32m " << (int)successful << " PASSED\033[0m\t\t" << std::endl;
    std::cout << "\x1B[31m " << (int)failed << " FAILED\033[0m\t\t" << std::endl;
}