#include <Eigen/Dense>
#include <chrono>
#include <iostream>
#include "params.hpp"
#include "./milp/mad_max/best_solution.hpp"
#include "./milp/mad_max/node.hpp"
#include "types.hpp"

using namespace std;
using namespace Eigen;


//for random test
int main() {
    int NN = 50;
    MatrixXd TimeSpent = MatrixXd::Zero(1, NN);
    MatrixXd Iter01proz = MatrixXd::Zero(1, NN);
    MatrixXd TimeSpent01proz = MatrixXd::Zero(1, NN);
    MatrixXd RelGap = MatrixXd::Zero(1, NN);
    MatrixXd NumIter = MatrixXd::Zero(1, NN);
    MatrixXd OptValueLow = MatrixXd::Zero(1, NN);
    MatrixXd NUMshares = MatrixXd::Zero(1, NN);
    MatrixXd TIMEhorizon = MatrixXd::Zero(1, NN);
    MatrixXd GammaMAD = MatrixXd::Zero(1, NN);

    int Nmax = 100; // maximal number of iterations

    for(int num_problem = 0; num_problem < NN; num_problem++) {
        MatrixXd historyUpperBound = MatrixXd::Zero(1, Nmax);
        MatrixXd historyLowerBound = MatrixXd::Zero(1, Nmax);
        // create parameters
        // not all parameters are used for MAD
        CommonParams* params = new CommonParams();
        params->setRandomParams();
        CommonParams_MADmax* commonParametersMax = new CommonParams_MADmax(params);
        /*std::cout << commonParametersMax->params->ind_shares << std::endl;
        std::cout << commonParametersMax->params->T << std::endl;
        std::cout << commonParametersMax->params->p_max << std::endl;
        std::cout << commonParametersMax->params->w_min << std::endl;
        std::cout << commonParametersMax->gamma_mad << std::endl;*/
        NUMshares(num_problem) = commonParametersMax->params->numShares;
        TIMEhorizon(num_problem) = commonParametersMax->params->T;
        MatrixXd historyNu = MatrixXd::Zero(commonParametersMax->params->numShares,Nmax);

        // create best integer solution
        // here we start with the empty solution with value +Inf
        auto start = std::chrono::high_resolution_clock::now();
        BestIntegerSolution* best_solution = new BestIntegerSolution(double_inf, VectorXi32::Zero(commonParametersMax->params->nu_max.size()));
        double old_best_value = best_solution->upperBound;
        //create root node
        VectorXi32 nu_lower = VectorXi32::Zero(commonParametersMax->params->numShares);
        VectorXi32 nu_upper = commonParametersMax->params->nu_max;
        Node* rootNode = new Node(commonParametersMax,nu_lower,nu_upper,best_solution,true,0);
        //starting iterations until lower bound matches upper bound
        //this happens when the root node closes
        std::clog << "Time horizon: " << commonParametersMax->params->T 
                  << ", Number of shares: " << commonParametersMax->params->numShares 
                  << ", w_min = " << commonParametersMax->params->w_min
                  << ", gammaMAD = " << commonParametersMax->gamma_mad
                  << std::endl;
        std::cout << "NumIter | Upper Bound | Lower Bound | Local Integer Solution | # integer values |  branch index | depth\n";
        int countIter = 0;
        double relGap = double_inf;
        while(rootNode->isNodeOpen && countIter < Nmax && (std::isinf(best_solution->upperBound) || relGap > 0.0001)) {
            //branch at node with minimal LP relaxation value
            Node* minNode = findMin(rootNode);
            double rV = minNode->roundValue; // when this is printed the node might no more exist
            VectorXd hN = minNode->nuRelax;
            int nDpth = minNode->depth;
            double oldval = minNode->LowerBound;
            int num_integer_values = bifurcate(minNode);
            uint bfI = minNode->BifurcationIndex;
            if(oldval > rootNode->LowerBound) {
                //std::cerr << "ERROR: oldval(" << oldval <<") > upperBound(" << rootNode->LowerBound << ")\n";
                //break;
            }

            if(old_best_value > best_solution->upperBound) {
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
            historyUpperBound(countIter) = -best_solution->upperBound;
            historyLowerBound(countIter) = -rootNode->LowerBound;
            historyNu.col(countIter) = hN;

            if(!std::isinf(best_solution->upperBound) && rootNode->LowerBound*best_solution->upperBound > 0) {
                relGap = rootNode->LowerBound/best_solution->upperBound - 1;
            }
            if(Iter01proz(num_problem) == 0 && relGap < 0.0001) {
                Iter01proz(num_problem) = countIter;
                
                auto finish = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> elapsed = finish - start;
                TimeSpent01proz(num_problem) = elapsed.count();
            }
        }
        if(rootNode->isNodeOpen)
            if(relGap < 0.0001 && !(best_solution->bestNu.size() == 0))
                std::clog << "Relative gap of 0.0001 achieved in " << countIter << " iterations\n";
            else 
                std::clog << "Maximal number of iterations achieved\n";
        else
            std::clog << "Optimal value: " << -best_solution->upperBound << std::endl;
        
        if (!(best_solution->bestNu.size() == 0)) {
            double MADvar = computeMADvariance(commonParametersMax, best_solution->bestNu.cast<double>());
            std::clog << "Variance of best achieved solution: " << MADvar << std::endl;
        } else {
            std::clog << "No feasible integer point found\n";
        }

        OptValueLow(num_problem) = -rootNode->LowerBound;
        GammaMAD(num_problem) = commonParametersMax->gamma_mad;

        delete commonParametersMax;
        delete best_solution;
        delete rootNode;
        auto finish = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = finish - start;
        std::clog << "Elapsed time for " << num_problem <<" problem: " << elapsed.count() << " s.\n";
        TimeSpent(num_problem) = elapsed.count();
        RelGap(num_problem) = relGap; 
        NumIter(num_problem) = countIter;
        std::cout << std::endl;
    } 
}