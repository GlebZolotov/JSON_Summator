#include <Eigen/Dense>
#include <chrono>
#include <iostream>
#include <algorithm>

#include "milp/MADmax_solver.hpp"
#include "params.hpp"
#include "milp/mad_max/best_solution.hpp"
#include "milp/mad_max/node.hpp"
#include "milp/mad_statement.hpp"
//#include "../types.hpp"

using namespace Eigen;

CommonParams_MADmax* getMaxCommonParameters(MadStatement* s) {
    CommonParams* params = new CommonParams();
    
    params->numShares = s->timeRate.rows();
    params->p_max = s->maxWeight(0);
    params->T = s->timeRate.cols();
    params->w_min = s->minSize;
    params->R = s->timeRate.transpose();
    //form Gr vector 
    params->Gr.resize(s->close.size());
    for(int i = 0; i < s->close.size(); ++i)
        params->Gr(i) = s->close(i) * s->lotSize(i) / s->capital;
    
    //multiply on granulates
    for(int i = 0; i < params->T; ++i)
        for(int j = 0; j < params->numShares; ++j)
            params->R(i, j) *= params->Gr(j);

    params->rbar =  params->R.colwise().mean(); //s->meanRate; 
    params->nu_max = floor((VectorXd::Constant(params->numShares, params->p_max)).cwiseQuotient(params->Gr).array()).cast<int32_t>();

    params->indGr.resize(params->Gr.size());
    std::size_t n(0);
    std::generate(params->indGr.begin(), params->indGr.end(), [&]{ return n++; });
    std::sort(params->indGr.begin(), params->indGr.end(),
            [&](int i1, int i2) { return params->Gr[i1] >= params->Gr[i2]; } );
    
    //params->setRandomParams();
    CommonParams_MADmax* max_params = new CommonParams_MADmax(params);
    max_params->gamma_mad = s->maxRisk;
    return max_params; 
}

MADMax_Solver::MADMax_Solver(MadStatement *st) 
: m_statement(st)
{
    commonParametersMax = getMaxCommonParameters(st);
}


MadSolution& MADMax_Solver::Solve()
{
    m_solution.status = SolutionStatus::NOT_STARTED;
    m_solution.returnValue = m_statement->maxRisk;
    m_solution.problemType = ProblemType::MAD_MAX;

    int Nmax = m_statement->iterationLimit;
    double timeLimit = m_statement->timeLimit;
    int nodeLimit = m_statement->nodeLimit;

    // create best integer m_solution
    // here we start with the empty m_solution with value +Inf
    auto start = std::chrono::high_resolution_clock::now();
    BestIntegerSolution *best_solution = new BestIntegerSolution(double_inf, VectorXi32::Zero(commonParametersMax->params->nu_max.size()));
    double old_best_value = best_solution->upperBound;
    //create root node
    VectorXi32 nu_lower = VectorXi32::Zero(commonParametersMax->params->numShares);
    VectorXi32 nu_upper = commonParametersMax->params->nu_max;
    Node *rootNode = new Node(commonParametersMax, nu_lower, nu_upper, best_solution, true, 0);
    //starting iterations until lower bound matches upper bound
    //this happens when the root node closes
    std::clog << "Time horizon: " << commonParametersMax->params->T
              << ", Number of shares: " << commonParametersMax->params->numShares
              << ", w_min = " << commonParametersMax->params->w_min
              << ", gammaMAD = " << commonParametersMax->gamma_mad
              << std::endl;
    std::cout << "NumIter | Upper Bound | Lower Bound | Local Integer m_solution | # integer values |  branch index | depth\n";
    int countIter = 0;
    double relGap = double_inf;
    double lastMeasuredTime = 0;
    int nodesCreated = 0;
    m_solution.status = SolutionStatus::STARTED; // here or on root ?
    while (rootNode->isNodeOpen && (std::isinf(best_solution->upperBound) || relGap > 0.0001))
    {
        if(countIter >= Nmax || lastMeasuredTime >= timeLimit || nodesCreated >= nodeLimit) {
            std::cerr << "Upbound input limits\n";
            break;
        }

        //branch at node with minimal LP relaxation value
        Node *minNode = findMin(rootNode);
        double rV = minNode->roundValue; // when this is printed the node might no more exist
        VectorXd hN = minNode->nuRelax;
        int nDpth = minNode->depth;
        double oldval = minNode->LowerBound;
        int num_integer_values = bifurcate(minNode);
        uint bfI = minNode->BifurcationIndex;
        if (oldval > rootNode->LowerBound)
        {
            //std::cerr << "ERROR: oldval(" << oldval <<") > upperBound(" << rootNode->LowerBound << ")\n";
            //break;
        }

        if (old_best_value > best_solution->upperBound)
        {
            old_best_value = best_solution->upperBound;
            updateByUpperBound(rootNode);
        }
        countIter++;
        std::clog << countIter << " \t| "
                  << -rootNode->LowerBound << "  | "
                  << -best_solution->upperBound << "  | "
                  << -rV << "\t| "
                  << num_integer_values << "\t| "
                  << bfI << "\t| "
                  << nDpth << "\t| "
                  << std::endl;

        if (!std::isinf(best_solution->upperBound) && rootNode->LowerBound * best_solution->upperBound > 0)
        {
            relGap = rootNode->LowerBound / best_solution->upperBound - 1;
        }
        
        auto finish = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = finish - start;
        timeLimit = elapsed.count();
        nodesCreated = bfI;

        m_solution.nodeAmount = nodesCreated;
        m_solution.iterationAmount = countIter;
        m_solution.time = timeLimit;
        m_solution.accuracy = relGap;
    }

    if (rootNode->isNodeOpen)
        if (relGap < 0.0001 && !(best_solution->bestNu.size() == 0))
            std::clog << "Relative gap of 0.0001 achieved in " << countIter << " iterations\n";
        else
            std::clog << "Maximal number of iterations achieved\n";
    else
        std::clog << "Optimal value: " << -best_solution->upperBound << std::endl;

    if (!(best_solution->bestNu.size() == 0))
    {
        double MADvar = computeMADvariance(commonParametersMax, best_solution->bestNu.cast<double>());
        std::clog << "Variance of best achieved m_solution: " << MADvar << std::endl;
    }
    else
    {
        m_solution.status = SolutionStatus::INCORRECT_PROBLEM;
        std::clog << "No feasible integer point found\n";
    }

    if(countIter >= Nmax) {
            m_solution.status = SolutionStatus::ITERATION_LIMIT_REACHED;
        }

    if(lastMeasuredTime >= timeLimit) {
            m_solution.status = SolutionStatus::TIME_LIMIT_REACHED;
        }

    if(nodesCreated >= nodeLimit) {
            m_solution.status = SolutionStatus::NODE_LIMIT_REACHED;
        }


    m_solution.numLots = best_solution->bestNu;
    m_solution.weights.resize(commonParametersMax->params->numShares);
    for(int i=0; i < best_solution->bestNu.size(); ++i)
        m_solution.weights(i) = best_solution->bestNu.cast<double>()(i)*commonParametersMax->params->Gr(i);
    m_solution.objectiveValue = -best_solution->upperBound;
    m_solution.numLots = m_statement->lotSize;

    delete best_solution;
    delete rootNode;
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::clog << "Elapsed time " << elapsed.count() << " s.\n";
    std::cout << std::endl; 
    return Current();
}

void MADMax_Solver::Interrupt() 
{

}

MadSolution& MADMax_Solver::Current() {
    return m_solution;
}
