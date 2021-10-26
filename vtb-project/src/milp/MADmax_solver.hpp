#pragma once
#include "../solver.hpp"
#include "./mad_solution.hpp"


class CommonParams_MADmax;
class MadStatement;

class MADMax_Solver : public Solver<MadSolution> 
{
public:
    MADMax_Solver(MadStatement*);

    virtual MadSolution& Solve();

    virtual void Interrupt();

    virtual MadSolution& Current();

private:
    CommonParams_MADmax* commonParametersMax;
    MadStatement* m_statement;
    MadSolution m_solution;

};