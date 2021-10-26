% main program for minimizing MAD variance
close all
clear all

rng(1); % err on Solution phase: 1750 simplex iterations
rng(2);

NN = 10;
TimeSpent = zeros(1,NN);
Iter01proz = zeros(1,NN);
TimeSpent01proz = zeros(1,NN);
RelGap = zeros(1,NN);
NumIter = zeros(1,NN);
NUMshares = zeros(1,NN);
TIMEhorizon = zeros(1,NN);
Nmax = 50; % maximal number of iterations
for num_problem = 1:NN
fprintf("Solving problem %d\n",num_problem);
historyUpperBound = zeros(1,Nmax);
historyLowerBound = zeros(1,Nmax);
% create parameters
% not all parameters are used for MAD
commonParameters = create_random_instance;
NUMshares(num_problem) = commonParameters.numshares;
TIMEhorizon(num_problem) = commonParameters.T;
historyNu = zeros(commonParameters.numshares,Nmax);
historyBifurcationIndex = zeros(1,Nmax);
historyBifurcationValue = zeros(1,Nmax);
% create best integer solution
% here we start with the empty solution with value +Inf
tic,
best_solution = BestIntegerSolution;
setValues(best_solution,+Inf);
old_best_value = best_solution.upperBound;
% create root node
nu_lower = uint16(zeros(commonParameters.numshares,1));
nu_upper = commonParameters.numax;
rootNode = NodeMADmin(commonParameters,nu_lower,nu_upper,best_solution,true,0);
% starting iterations
% until lower bound matches upper bound
% this happens when the root node closes
fprintf("Time horizon: %d, Number of shares: %d, w_min = %5e\n",commonParameters.T,commonParameters.numshares,commonParameters.wmin);
fprintf("NumIter | Lower Bound  | Upper Bound  | Local Integer Solution | # integer values | branch index | depth\n");
countIter = 0;
relGap = +Inf;
while rootNode.nodeOpen && (countIter < Nmax) && (isinf(best_solution.upperBound) || (relGap > 0.0001))
    % branch at node with minimal LP relaxation value
    minNode = findMin(rootNode);
    rV = minNode.roundValue; % when this is printed the node might no more exist
    hN = minNode.nuRelax;
    nDpth = minNode.depth;
    n0 = length(minNode.determined_nu);
    n1 = length(find(minNode.LowerNuBound + 1 == minNode.UpperNuBound));
    oldval = rootNode.LowerBound;
    [num_integer_values,bfI,bfV] = bifurcate(minNode);
    if oldval > rootNode.LowerBound
        disp('error') % this is be able to analyze the problem if this case occurs (probably this was due to the numerical errors in the cvx solution)
    end
    if old_best_value > best_solution.upperBound
        old_best_value = best_solution.upperBound;
        updateByUpperBound(rootNode);
    end
    countIter = countIter + 1;
    fprintf("  %5d | %5e | %5e |           %5e |        %5d    |        %5d    |        %5d     |     %5d    | %5d\n",countIter,rootNode.LowerBound,best_solution.upperBound,rV,n0,n1,num_integer_values,bfI,nDpth);
    historyUpperBound(countIter) = best_solution.upperBound;
    historyLowerBound(countIter) = rootNode.LowerBound;
    historyNu(:,countIter) = hN;
    historyBifurcationIndex(countIter) = bfI;
    historyBifurcationValue(countIter) = bfV;
    if ~isinf(best_solution.upperBound)
        relGap = best_solution.upperBound/rootNode.LowerBound - 1;
    end
    if (Iter01proz(num_problem) == 0) && (relGap < 0.001) % clock when 0.1% gap is reached
        Iter01proz(num_problem) = countIter;
        TimeSpent01proz(num_problem) = toc;
    end
end
% output result
if rootNode.nodeOpen
    if (relGap < 0.0001) && (~isempty(best_solution.bestNu))
        fprintf("Relative gap of 0.0001 achieved in %d iterations\n",countIter);
    else
        fprintf("Maximal number of iterations achieved\n");
    end
else
    fprintf("Optimal value: %5e\n",best_solution.upperBound);
end
if isempty(best_solution.bestNu)
    fprintf("No feasible integer point found")
end
% clear objects
delete(rootNode);
delete(best_solution);
delete(commonParameters);
timeSpent = toc;
fprintf("Time spent: %5e sec\n\n",timeSpent);
TimeSpent(num_problem) = timeSpent;
RelGap(num_problem) = relGap;
NumIter(num_problem) = countIter;
end
k = NN;
csvwrite('MADmin_tmp.csv',[1:k; TIMEhorizon(1:k); NUMshares(1:k); TimeSpent01proz(1:k); TimeSpent(1:k); RelGap(1:k); Iter01proz(1:k); NumIter(1:k)]');

% figure
% plot((1:countIter)/countIter*timeSpent,historyUpperBound(1:countIter))
% hold on
% plot((1:countIter)/countIter*timeSpent,historyLowerBound(1:countIter))
% xlabel('time (sec)')
% title('Upper and lower bounds')
% grid on
% figure
% plot((1:countIter)/countIter*timeSpent,log(max(zeros(1,countIter),historyUpperBound(1:countIter)./historyLowerBound(1:countIter) - 1))/log(10))
% title('Relative Log Gap (Basis 10)')
% xlabel('time (sec)')
% grid on
