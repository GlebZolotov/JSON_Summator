// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Roland Hildebrand <roland.hildebrand@univ-grenoble-alpes.fr>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#pragma once

#include "../../solver.hpp"
#include "../mad_solution.hpp"
#include "../mad_statement.hpp"

class MADMin_Solver : public Solver<MadSolution>
{
public:
    MADMin_Solver(MadStatement* st);

    MadSolution&
    Solve() override;

    void
    Interrupt() override;

    MadSolution&
    Current() override;

private:
    MadStatement* m_statement;
    MadSolution m_solution;
};
