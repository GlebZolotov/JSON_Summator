function [s,nu] = MADmax_feas(commonParameters)
% function [s,nu] = MADmax_feas(commonParameters)
%
% checks feasibility of MAD max problem
% s is a boolean indicating whether the problem is feasible
% if it is, nu is an integer feasible point
Nmax = 100; % maximal number of iterations
% create best integer solution
% here we start with the empty solution with value +Inf
best_solution = BestIntegerSolution;
setValues(best_solution,+Inf);
old_best_value = best_solution.upperBound;
% create root node
nu_lower = uint16(zeros(commonParameters.numshares,1));
nu_upper = commonParameters.numax;
rootNode = NodeClassMADmaxfeas(commonParameters,nu_lower,nu_upper,best_solution,true,0);
% starting iterations
% until lower bound matches upper bound
% this happens when the root node closes
fprintf("Time horizon: %d, Number of shares: %d, w_min = %5e, gamma_MAD = %5e\n",commonParameters.T,commonParameters.numshares,commonParameters.wmin,commonParameters.gammaMAD);
fprintf("MAD max feasibility check\n");
fprintf("NumIter | Lower Bound  | Upper Bound  | Local Integer Solution | # fixed values | # binary values | # integer values | branch index | depth\n");
countIter = 0;
constraint_value = commonParameters.gammaMAD*double(commonParameters.T)/2;
while rootNode.nodeOpen && (countIter < Nmax) && (rootNode.LowerBound <= constraint_value) && (best_solution.upperBound > constraint_value)
    % branch at node with minimal LP relaxation value
    minNode = findMin(rootNode);
    rV = minNode.roundValue; % when this is printed the node might no more exist
    bfI = minNode.BifurcationIndex;
    nDpth = minNode.depth;
    n0 = length(minNode.determined_nu);
    n1 = length(find(minNode.LowerNuBound + 1 == minNode.UpperNuBound));
    oldval = rootNode.LowerBound;
    num_integer_values = bifurcate(minNode);
    if oldval > rootNode.LowerBound
        disp('error') % this is to be able to analyze the problem if this case occurs (probably this was due to the numerical errors in the cvx solution)
    end
    if old_best_value > best_solution.upperBound
        old_best_value = best_solution.upperBound;
        updateByUpperBound(rootNode);
    end
    countIter = countIter + 1;
    fprintf("  %5d | %5e | %5e |           %5e |        %5d    |        %5d    |        %5d     |     %5d    | %5d\n",countIter,rootNode.LowerBound,best_solution.upperBound,rV,n0,n1,num_integer_values,bfI,nDpth);
end
% output result
if best_solution.upperBound <= constraint_value
    fprintf("Feasible solution found in %d iterations\n",countIter);
    nu = best_solution.bestNu;
    s = true;
elseif rootNode.LowerBound > constraint_value
    fprintf("Infeasibility certified in %d iterations\n",countIter);
    nu = [];
    s = false;
else
    fprintf("Maximal number of iterations achieved\n");
    s = false; % this is not treated separately at the moment
end
% clear objects
delete(rootNode);
delete(best_solution);
