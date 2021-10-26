#include <Eigen/Dense>
#include "simplex.hpp"
#include <iostream>

using namespace Eigen;
using namespace std;
using namespace solver;


bool test_feasible()
{
    // number of natural variables
    size_t n0 = 2;
    // total number of variables 
    size_t n = 7;
    // number of equations
    size_t m = 5;

    MatrixXf A;
    A.resize(m, n);

    // matrix defining the constraints
    A << 2, 1, 0, 0, 0, 0, 1,
        1, 0, 0, 0, -1, 0, 0,
        0, 1, 0, 0, 0, -1, 0,
        1, 0, 1, 0, 0, 0, 0,
        0, 1, 0, 1, 0, 0, 0;

    // boundary values
    float l = 0, u = 3;

    // rhs of non-boundary equations 
    VectorXf b0;
    b0.resize(1);
    b0 << 6;

    // coefficients defining the problem being solved
    VectorXf c;
    c.resize(n);
    c << -1, -2, 0, 0, 0, 0, 0;

    // starting feasible point (is not trivial in general)
    VectorXf x_0 = VectorXf::Zero(n0);

    // root solver initaialization, matrix formation
    auto solver = SimplexSolver(n, n0, m, l, u, A, b0, c, x_0, Eigen::VectorXi(0), true);

    // obtaining of the solution for the relaxation
    StatusReturn feasible_point = solver.get_feasible();
    if (feasible_point.status_code == 0)
    {
        std::cout << "x =\n" << feasible_point.result << "\n";
        return true;
    }
    else
    {
        return false;
    }
}


bool test_demo_problem()
{
    // number of natural variables
    size_t n0 = 2;
    // total number of variables 
    size_t n = 7;
    // number of equations
    size_t m = 5;

    MatrixXf A;
    A.resize(m, n);

    // matrix defining the constraints
    A << 2, 1, 0, 0, 0, 0, 1,
        1, 0, 0, 0, -1, 0, 0,
        0, 1, 0, 0, 0, -1, 0,
        1, 0, 1, 0, 0, 0, 0,
        0, 1, 0, 1, 0, 0, 0;

    // boundary values
    float l = 0, u = 3;

    // rhs of non-boundary equations 
    VectorXf b0;
    b0.resize(1);
    b0 << 6;

    // coefficients defining the problem being solved
    VectorXf c;
    c.resize(n);
    c << -1, -2, 0, 0, 0, 0, 0;

    // starting feasible point (is not trivial in general)
    VectorXf x_0 = VectorXf::Zero(n0);

    // root solver initaialization, matrix formation
    auto solver = SimplexSolver(n, n0, m, l, u, A, b0, c, x_0);

    // obtaining of the solution for the relaxation
    LPSolution yet_another = solver.solve_instance();
    std::cout << "Root f = " << yet_another.f << "\n";
    if (yet_another.f != -7.5)
    {
        return false;
    }

    // detection of non-integer components and preprocesssing for splitting
    int split_i = solver.split(yet_another);
    if (split_i == -1)
    {
        std::cout << "Result is f = " << yet_another.f;
        return false;
    }
    else
    {
        // appending first leaf with lower variable split, obtaining solution
        LPSolution left_one = solver.get_left_leaf()->solve_instance();
        std::cout << "Left leaf f = " << left_one.f << "\n";
        if (left_one.f != -7)
        {
            return false;
        }

        // the same with upper split
        LPSolution right_one = solver.get_right_leaf()->solve_instance();
        std::cout << "Right leaf f = " << right_one.f << "\n";
        if (right_one.f != -6)
        {
            return false;
        }

        // choose the best, insofar as in this case two stages are sufficient
        if (left_one.f < right_one.f)
        {
            std::cout << "Result is f = " << left_one.f;
            return true;
        }
        else
        {
            std::cout << "Result is f = " << right_one.f;
            return false;
        }
    }
}


bool test_presolve_phases()
{
    Eigen::VectorXi B = Eigen::VectorXi();
    B.resize(1);
    B[0] = 2;
    
    Eigen::MatrixXf M = Eigen::MatrixXf(2, 4);
    M << 1, 10, 0, -100,
         1,  2, 1, 3;

    auto solver = SimplexSolver(M, B);
    solver.iterate_presolved_leaf_simplex(true);

    return true;
}


int main()
{
    std::cout << "\n------------=== test 1 : demo     ===------------\n";
    assert(test_demo_problem());
    std::cout << "\n------------=== test 2 : phases   ===------------\n";
    assert(test_presolve_phases());
    std::cout << "\n------------=== test 3 : feasible ===------------\n";
    assert(test_feasible());

    return 0;
}