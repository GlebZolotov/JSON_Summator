#pragma once

#include <Eigen/Dense>
#include <limits>
#include <iostream>
#include <any>

#include "utils.hpp"

namespace solver
{
    using namespace Utils;

    struct LPSolution
    {
        // auxiliary structure to store both point and value it gives
        Eigen::VectorXf x;
        float f;
    };

    struct StatusReturn
    {
        char status_code;
        Eigen::VectorXf result;
    };

    struct SimplexSolver
    {
    private:
        // dimensionalities of the problem
        size_t n, n0, m;

        // pointers defining tree-formed structure of the solver
        // for efficient matrix propagation
        SimplexSolver *parent;
        bool is_left;

        SimplexSolver *left_leaf;
        SimplexSolver *right_leaf;

        // dynamic vectors of componentwise boundary values
        Eigen::VectorXf l;
        Eigen::VectorXf u;

        // boundary values changed in `split` procedure
        // needed to perform calls for left / right node 
        // with (l, u_new) / (u, l_new) bounds combinations 
        Eigen::VectorXf l_new;
        Eigen::VectorXf u_new;

        // main matrix for simplex operation
        Eigen::MatrixXf M;
        // ordered sequence of base variables (order is important for impl.)
        Eigen::VectorXi B;
        // ordered sequence of non-base variables (order is important for impl.)
        Eigen::VectorXi NB;

        // change in only modified variable defined in `split` procedure
        float delta = 0.0;
        Eigen::VectorXf delta_presolve;
        // split index to give to leaf nodes
        int split_i = -1;

        bool find_feasible;

        // ctor for first leaf (copying M and B)
        // (here and further ctor args are passed by value with move trick, on average it's okey)
        SimplexSolver(
            size_t n, size_t n0, size_t m,
            Eigen::VectorXf l, Eigen::VectorXf u,
            Eigen::MatrixXf const &M, Eigen::VectorXi const &B,
            SimplexSolver *parent)
            : n(n), n0(n0), m(m), parent(parent), left_leaf(nullptr), right_leaf(nullptr),
              l(std::move(l)), u(std::move(u)),
              M(M), B(B) {}

        // ctor for second leaf (moving M and B)
        SimplexSolver(
            size_t n, size_t n0, size_t m,
            Eigen::VectorXf l, Eigen::VectorXf u,
            Eigen::MatrixXf &&M, Eigen::VectorXi &&B,
            SimplexSolver *parent, bool is_left)
            : n(n), n0(n0), m(m), parent(parent), is_left(is_left), 
              left_leaf(nullptr), right_leaf(nullptr),
              l(std::move(l)), u(std::move(u)),
              M(std::move(M)), B(std::move(B)) {}

    public:
        SimplexSolver(
            size_t n, size_t n0, size_t m,
            Eigen::VectorXf l, Eigen::VectorXf u,
            Eigen::MatrixXf A, Eigen::VectorXf b0, Eigen::VectorXf c, 
            Eigen::VectorXf x_0, Eigen::VectorXi base = Eigen::VectorXi(0),
            bool find_feasible=false)
            : n(n), n0(n0), m(m), parent(nullptr), is_left(false), 
              left_leaf(nullptr), right_leaf(nullptr),
              l(l), u(u), B(base)
        {
            // main ctor for root node of solver

            // full last column for main table
            Eigen::VectorXf b(b0.size() + l.size() * 2);
            // (order is important)
            b << b0, l, u;

            this->find_feasible = find_feasible;
            if (find_feasible)
            {
                Eigen::VectorXf coef_feasible = Eigen::VectorXf::Zero(n);
                coef_feasible.tail(n - n0) = Utils::full_of(n - n0, 1.0);

                Eigen::VectorXf start_feasible = Eigen::VectorXf::Zero(n);
                start_feasible.tail(n - n0) = b;

                M = Eigen::MatrixXf(m + 1, n + 1);
                M.row(0).head(n) = coef_feasible;
                M(0, n) = coef_feasible.dot(start_feasible);
                M.block(1, 0, m, n) = A;
                M.col(n).tail(m) = b;

                B = Eigen::VectorXi(n - n0);
                for (size_t i = 0; i < B.size(); ++i)
                {
                    B[i] = n0 + i;
                }
            }
            else
            {
                // initialization of extended point with boundary and natural residuals
                Eigen::VectorXf x = x_0;
                Eigen::VectorXf y(n);
                y << x, u - x, x - l, b0;

                // main matrix with corresponding blocks
                M = Eigen::MatrixXf(m + 1, n + 1);
                M.row(0).head(n) = c;
                M(0, n) = c.dot(y);
                M.block(1, 0, m, n) = A;
                M.col(n).tail(m) = b;

                // define starting base variables
                find_base();
            }

            // transform matrix to a standard form with one matrix inversion
            Eigen::MatrixXf Inv(m + 1, m + 1);
            for (int i = 0; i < B.size(); ++i)
            {
                Inv.col(i + 1) = M.col(B[i]);
            }
            Inv.col(0).setZero();
            Inv(0, 0) = 1;
            Inv = Inv.inverse();
            M = Inv * M;

            // eliminate redundand variables in matrix
            eliminate_x();
        }

        // auxiliary ctor with all the same boundary values
        SimplexSolver(
            size_t n, size_t n0, size_t m,
            float l, float u,
            Eigen::MatrixXf A, Eigen::VectorXf b0, Eigen::VectorXf c, 
            Eigen::VectorXf x_0, Eigen::VectorXi base = Eigen::VectorXi(0),
            bool find_feasible=false)
            : SimplexSolver(n, n0, m, full_of(n0, l), full_of(n0, u), A, b0, c, x_0, base, find_feasible) {}

        // TODO: remove
        // to test
        SimplexSolver(Eigen::MatrixXf const &M, Eigen::VectorXi const &B)
            : M(M), B(B) 
        {
            m = M.rows() - 1;
            n = M.cols() - 1;
            n0 = 2;

            parent = nullptr;
            left_leaf = nullptr;
            right_leaf = nullptr;

            delta_presolve = Eigen::VectorXf();
            delta_presolve.resize(3);
            delta_presolve << 0.5, 0.5, 0.5;
        }

        ~SimplexSolver()
        {
            if (left_leaf != nullptr)
            {
                delete left_leaf;
            }
            if (right_leaf != nullptr)
            {
                delete right_leaf;
            }
        }

        LPSolution solve_instance()
        {
            // main interface procedure, redirecting to the right iterative impl.
            // and returning the solution for current perticular leaf

            if (parent == nullptr)
            {
                iterate_simplex();
            }
            else
            {
                iterate_leaf_simplex();
            }

            return solution();
        }

        // TODO: Add exception for split_i == -1
        // TODO: think on moves for `u`
        SimplexSolver* get_left_leaf()
        {
            // get or init first leaf with copied matrix

            if (left_leaf == nullptr)
            {
                left_leaf = new SimplexSolver(n, n0, m, l, u_new, M, B, this);
                left_leaf->delta = u_new[split_i] - u[split_i];
                // flag is important due to the specific variables order
                left_leaf->is_left = true;
            }
            return left_leaf;
        }

        SimplexSolver* get_right_leaf()
        {
            // get or init second leaf with moved matrix

            if (right_leaf == nullptr)
            {
                right_leaf = new SimplexSolver(n, n0, m, l_new, std::move(u), std::move(M), std::move(B), this);
                right_leaf->delta = l[split_i] - l_new[split_i];
            }
            return right_leaf;
        }

        StatusReturn get_feasible()
        {
            if (find_feasible)
            {
                std::cout << "Feasible point phase" << "\n";
                LPSolution feasible = solve_instance();
                if (feasible.f == 0)
                {
                    std::cout << "get done!" << std::endl;
                }
                return {0, feasible.x.head(n0)};
            }
            else
            {
                return {1, Eigen::VectorXf::Zero(n0)};
            }
        }

        LPSolution solve()
        {
            // full solving procedure, which probably will not be used,
            // but possible to be called for every node as subtree descent

            // get for parent
            LPSolution yet_another = solve_instance();

            // check integer feasibility
            int split_i = split(yet_another);
            if (split_i == -1)
            {
                return yet_another;
            }

            // branch if failed
            LPSolution left_one = get_left_leaf()->solve_instance();
            LPSolution right_one = get_right_leaf()->solve_instance();

            // choose the best
            if (left_one.f < right_one.f)
            {
                return left_one;
            }
            else
            {
                return right_one;
            }
        }

        int split(LPSolution const &sol)
        {
            // splitting preprocessing procedure
            // checking integer feasibility and preparing
            // new boundary values for branching

            split_i = -1;

            for (int i = 0; i < sol.x.size(); ++i)
            {
                if (ceilf(sol.x[i]) != sol.x[i])
                {
                    split_i = i;
                    break;
                }
            }

            if (split_i == -1)
            {
                return split_i;
            }

            l_new = Eigen::VectorXf(n0);
            u_new = Eigen::VectorXf(n0);
            l_new = l;
            u_new = u;

            l_new[split_i] = ceilf(sol.x[split_i]);
            u_new[split_i] = floorf(sol.x[split_i]);

            return split_i;
        }

    private:
        LPSolution solution() const
        {
            // recover the solution from constricted matrix

            Eigen::VectorXf x(n0);
            x.setZero();

            for (size_t i = 0; i < B.size(); ++i)
            {
                size_t b = B[i];
                // the explanation of importance of the order
                // x`s (n0) are eliminated
                // s+`s (n0) are first
                // s-`s (n0) are second
                // so we check if next base variable is s-, take it...   
                if (2 * n0 <= b + n0 && b + n0 < 3 * n0)
                {
                    x[b - n0] = M(i + 1, n);
                }
            }
            // ... and recover the point as x = l + s-
            x += l;
            return {x, -M(0, n)};
        }

        size_t iterate_simplex(size_t additional_line = 0)
        {
            // iterative procedure itself
            size_t iters = 0;

            // `additional_line` flag is for the leaf nodes, 
            // where we zeroing target of additional line.
            // in the root the condition is the positivity of sensitivities
            while ((additional_line == 0 && !all_non_negative(M.row(0))) || (additional_line > 0 && M(0, n) != 0))
            {
                ++iters;
                size_t j = argmin(M.row(0));

                Eigen::VectorXf nonzeros(m);
                for (size_t i = 1 + additional_line, k = 0; i < m + 1 + additional_line; ++i, ++k)
                {
                    if (M(i, j) > 0)
                    {
                        nonzeros[k] = M(i, n) / M(i, j);
                    }
                    else
                    {
                        nonzeros[k] = std::numeric_limits<float>::max();
                    }
                }

                size_t k = argmin(nonzeros);
                std::cout << "!" << NB.size() << " " << j << std::endl;
                // need to change indexing of columns
                // NB[j] = B[k];
                B[k] = j;


                M.row(k + 1 + additional_line) *= 1.0 / M(k + 1 + additional_line, j);
                for (size_t i = 0; i < m + 1 + additional_line; ++i)
                {
                    if (i != k + 1 + additional_line)
                    {
                        M.row(i) -= M(i, j) * M.row(k + 1 + additional_line);
                    }
                }

                std::cout << "M =\n" << M << std::endl;
                std::cout << "B =\n" << B << std::endl;
                std::cout << "NB =\n" << NB << std::endl;
            }

            std::cout << "  ∟ Iterations number = " << iters << "\n";
            return iters;
        }

        void iterate_leaf_simplex()
        {
            // envelope for leaf iterative procedure

            std::cout << "# Leaf" << "\n";
            // here the order is also important, we get changed s+ or s- (with shift of `n0`)
            size_t j = parent->split_i + (is_left ? 0 : n0);

            size_t q;
            for (size_t i = 1; i < m + 1; ++i)
            {
                if (M(i, j) == 1)
                {
                    q = i;
                }
            }

            M(q, n) += delta;
            M.row(q) *= -1.0;
            M.col(j) *= -1.0;

            Eigen::VectorXf q_dublicate = M.row(q) * -1.0;
            q_dublicate[j] = 0.0;

            // add new target line in matrix
            Eigen::MatrixXf tmp;
            tmp.resize(M.rows() + 1, M.cols());
            tmp.row(0) = q_dublicate;
            tmp.block(1, 0, m + 1, n + 1) = M;
            M = tmp;

            // call for phase 1
            std::cout << "Phase 1" << "\n";
            iterate_simplex(1);

            // delete additional line
            M = M.block(1, 0, m + 1, n + 1);
            M.col(j) *= -1;

            // enveloped call for phase 2
            std::cout << "Phase 2" << "\n";
            iterate_simplex();
        }

        void eliminate_x()
        {
            // delete x`s we can recover from other variables

            for (size_t i = 0; i < n0; ++i)
            {
                remove_row(M, 1);
                remove_column(M, 0);
            }

            B = B.tail(m - n0);
            for (int i = 0; i < B.size(); ++i)
            {
                B[i] -= n0;
            }

            // parameters of the problem changing too
            n -= n0;
            m -= n0;
        }

        void find_base()
        {
            // define the base variables of the starting point
            if (B.size() != m)
            {
                // now all the variables except s-`s are base, i'm not sure it's correct
                B = Eigen::VectorXi(m);
                for (size_t i = 0; i < n0; ++i)
                {
                    B[i] = i;
                    B[n0 + i] = n0 + i;
                }
                for (size_t i = 0; i < m - 2*n0; ++i)
                {
                    B[m - i - 1] = n - i - 1;
                }
            }

            NB = Eigen::VectorXi(n - m);
            for (size_t i = 0, b_iterator = 0, nb_iterator = 0; i < n; ++i)
            {
                if (B[b_iterator] == i)
                {
                    ++b_iterator;
                }
                else
                {
                    NB[nb_iterator] = i;
                }
            }
        }

    public:
        float iterate_presolved_leaf_simplex(bool test = false)
        {
            // envelope for leaf iterative procedure
            // when we have multiple delta`s after presolve

            std::cout << "# Leaf (multiple delta`s)" << "\n";

            Eigen::MatrixXf tmp;
            tmp.resize(M.rows() + 2, M.cols() + 2);
            tmp.block(1, 0, m + 1, n) = M.block(0, 0, m + 1, n);
            
            tmp.col(tmp.cols() - 1).segment(1, M.rows()) = M.col(M.cols() - 1);

            // column for y extensive variable
            tmp.col(M.cols() + 2 - 3).setZero();
            for (size_t i = 0; i < 1 + (m + 1); ++i)
            {
                tmp(i, M.cols() + 2 - 3) = tmp.row(i).head(M.cols() - 1).dot(delta_presolve);
            }

            // set column for z residual to zero.
            // order of further assignments is important
            // since this column is not zero eventually
            tmp.col(M.cols() + 2 - 2).setZero();

            tmp.row(m + 2).setZero();
            tmp(m + 2, M.cols() + 2 - 1) = 1;
            // here we modify values for z- and y-columns
            tmp(m + 2, M.cols() + 2 - 2) = 1;
            tmp(m + 2, M.cols() + 2 - 3) = 1;

            Eigen::VectorXf q_dublicate = tmp.row(M.rows() + 2 - 1) * -1.0;
            q_dublicate[M.cols() + 2 - 2] = 0.0;
            tmp.row(0) = q_dublicate;
            M = tmp;

            B.conservativeResize(B.size() + 1);
            B[B.size() - 1] = n + 2 - 1;

            ++m; n += 2;

            // // call for phase 1
            std::cout << "Phase 1" << "\n";
            iterate_simplex(1);

            // delete additional lines

            if (M(0, M.cols() - 2) == 1.0)
            {
                std::cout << "get done!" << std::endl;
            }

            bool exist_extended_base_vars = true;
            while (exist_extended_base_vars)
            {
                exist_extended_base_vars = false;
                for (int i = 0; i < B.size(); ++i)
                {
                    if (B[i] == M.cols() - 1 - 1 || B[i] == M.cols() - 2 - 1)
                    {
                        remove_element(B, i);
                        exist_extended_base_vars = true;
                        break;
                    }
                }
            }

            M = M.block(1, 0, m + 1, n + 1);

            remove_column(M, M.cols() - 2);
            remove_column(M, M.cols() - 2);
            --m; --n;

            if (!test)
            {
                // enveloped call for phase 2
                std::cout << "Phase 2" << "\n";
                iterate_simplex();
            }

            return M(0, M.cols() - 1);
        }
    };
}