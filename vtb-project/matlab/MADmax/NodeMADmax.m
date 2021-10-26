classdef NodeMADmax < handle
% this is the class definition for the MAD max problem
    
   properties
      
      LowerBound double
      LowerChildNode NodeMADmax
      UpperChildNode NodeMADmax
      ParentNode NodeMADmax
      LowerNuBound uint16 % lower bounds on integer variables
      UpperNuBound uint16 % upper bounds on integer variables
      BifurcationIndex uint16 % index of integer variable according to which child nodes are created
      BifurcationValueLow uint16 % lower integer value of branching variable
      BifurcationValueHigh uint16 % higher integer value of branching variable
      nodeOpen logical % determines whether the node has to be further investigated
      bestSolution BestIntegerSolution
      commonParams CommonParameters
      isRootNode logical % if true then this is the root node
      isLeaf = true % if true then there are no child nodes
      nuRelax double % vector of integer variables obtained from LP relaxation, entries may be non-integer
      roundValue double % value of integer point obtained by rounding procedure
      depth uint16 % depth of node in the tree, the root node has depth 0
      simplexTableau double % simplex tableau of the optimal solution of the LP relaxation
      basicSet uint16 % basic set of simplex tableau
      % the variables corresponding to the columns are:
      % slack (l <= nu), slack(nu <= u), slack(wmin <= Gr*nu),
      % slack(Gr*nu <= 1), slack(sum(b) <= gammaMAD*T/2), a, b
      non_determined_nu uint16 % index set of nu_i which are not fixed by lower and upper bounds
      non_determined_nu_logical logical
      determined_nu uint16 % index set of nu_i which are fixed by lower and upper bounds
      fracIndices uint16 % index set of fractional nu variables among the non_determined_nu
      
   end
   
   methods
       
       % CONSTRUCTOR
       % creates a new node
       % if the argument parentNode is empty, this is the root node
       % when it is created, the node solves its LP relaxation
       % if the value is above the value of the current best solution, the
       % node declares itself closed
       % if the LP solution is integer, then the node is also closed
       % if the integer solution is better than the current best, it
       % replaces the best solution
       % if the value is lower than the current best and not integer, the
       % node declares itself as open and computes the index of the
       % variable for branching
       function obj = NodeMADmax(commonParameters,nu_lower,nu_upper,best_solution,is_root_node,nodeDepth,parentNode)
           obj.bestSolution = best_solution;
           obj.commonParams = commonParameters;
           obj.isRootNode = is_root_node;
           obj.depth = nodeDepth;
           if ~is_root_node
               obj.ParentNode = parentNode;
               [nu_lower,nu_upper] = tighten_boundsMADmax(commonParameters,nu_lower,nu_upper);
               if min(double(nu_upper) - double(nu_lower)) < 0 % infeasible
                   obj.LowerBound = +Inf;
                   obj.nodeOpen = false;
                   obj.roundValue = +Inf;
                   return;
               elseif sum(abs(double(nu_upper) - double(nu_lower))) == 0 % only one integer point is potentially feasible
                   nu = double(nu_lower);
                   MADvar = computeMADvariance(commonParameters,nu);
                   wGr = commonParameters.Gr'*nu;
                   if (MADvar <= commonParameters.gammaMAD) && (commonParameters.wmin <= wGr) && (wGr <= 1) % point feasible
                       value = commonParameters.rbar*nu;
                       obj.nodeOpen = false;
                       obj.roundValue = -value;
                       obj.LowerBound = -value;
                       setValues(best_solution,-value,nu);
                       return;
                   else % point not feasible
                       obj.LowerBound = +Inf;
                       obj.nodeOpen = false;
                       obj.roundValue = +Inf;
                       return;
                   end
               end
           end
           obj.non_determined_nu_logical = (nu_lower ~= nu_upper);
           obj.non_determined_nu = find(obj.non_determined_nu_logical);
           obj.determined_nu = find(nu_lower == nu_upper);
           obj.LowerNuBound = nu_lower;
           obj.UpperNuBound = nu_upper;
           % solve LP relaxation
           [s,single_index,value,nu_low,nu_upp,nu,index] = MADmaximize_return_on_cube(commonParameters,nu_lower,nu_upper,obj);
           % save partial lower bound associated to the node
           if ~s
               obj.LowerBound = +Inf;
           else
               value = -value;
               obj.LowerBound = value;
               obj.nuRelax = nu;
               obj.fracIndices = find(abs(nu(obj.non_determined_nu) - round(nu(obj.non_determined_nu))) > 10^(-9));
           end
           % checks if node has higher value than the current best integer one
           if ~s || (best_solution.upperBound <= value) % node gives worse values than the best found so far
               obj.nodeOpen = false;
               obj.roundValue = +Inf;
           else
               % checks if node yields an integer solution
               % look for non-integer value in the solution with the highest
               % sensitivity of the cost function
               if single_index && (index == 0)
                   % obtained integer solution, it is better than the best
                   % solution
                   obj.nodeOpen = false;
                   obj.roundValue = value;
                   setValues(best_solution,value,nu);
               else
                   % obtained non-integer solution is better than current
                   % integer value
                   % check three integer points
                   [s,value,nu_local] = MADmax_integer_local_improve(commonParameters,nu_low);
                   if s
                       obj.roundValue = -value;
                       if -value < best_solution.upperBound
                           setValues(best_solution,-value,nu_local);
                       end
                   end
                   [s,value,nu_local] = MADmax_integer_local_improve(commonParameters,nu_upp);
                   if s
                       if -value < obj.roundValue
                           obj.roundValue = -value;
                       end
                       if -value < best_solution.upperBound
                           setValues(best_solution,-value,nu_local);
                       end
                   end
                   if ~single_index % round(nu) may be different from nu_low and nu_upp
                       [s,value,nu_local] = MADmax_integer_local_improve(commonParameters,round(nu));
                       if s
                           if -value < obj.roundValue
                               obj.roundValue = -value;
                           end
                           if -value < best_solution.upperBound
                               setValues(best_solution,-value,nu_local);
                           end
                       end
                   end
                   obj.nodeOpen = true;
               end
           end
       end
       
       % DESTRUCTOR
       function delete(node)
           if ~node.isLeaf
               delete(node.LowerChildNode);
               delete(node.UpperChildNode);
           end
       end
       
       % bifurcates by creating child nodes
       % uses the previously computed index and values of the branching
       % variable
       % gives back the number of integer values in the vector of integer
       % variables
       function [num_integer_values,bfI,bfV] = bifurcate(node)
           num_integer_values = length(find(node.nuRelax == round(node.nuRelax)));
           % determine branch variable
           % this is based on the sensitivities of the cost with respect to
           % changes in the fractional variables
           if ~isempty(node.simplexTableau) % we may estimate the sensitivities from the simplex table
               sensitivities = zeros(length(node.fracIndices),2);
               n = length(node.non_determined_nu);
               for k = 1:length(node.fracIndices)
                   bas_ind = find(node.basicSet == node.fracIndices(k), 1); % row of lower slack corresponding to fractional variable
                   negind = find(node.simplexTableau(bas_ind+1,1:end-1) < 0); % the basic columns contain either 0 or 1 in this row
                   if ~isempty(negind)
                       sensitivities(k,1) = min(-node.simplexTableau(1,negind)./node.simplexTableau(bas_ind+1,negind));
                   else
                       sensitivities(k,1) = Inf; % dual unbounded, primal infeasible
                   end
                   bas_ind = find(node.basicSet == n+node.fracIndices(k), 1); % row of upper slack corresponding to fractional variable
                   negind = find(node.simplexTableau(bas_ind+1,1:end-1) < 0);
                   if ~isempty(negind)
                       sensitivities(k,2) = min(-node.simplexTableau(1,negind)./node.simplexTableau(bas_ind+1,negind));
                   else
                       sensitivities(k,2) = Inf; % dual unbounded, primal infeasible
                   end
               end
               lower_gains = sensitivities(:,2).*(node.nuRelax(node.non_determined_nu(node.fracIndices)) - floor(node.nuRelax(node.non_determined_nu(node.fracIndices))));
               upper_gains = sensitivities(:,1).*(ceil(node.nuRelax(node.non_determined_nu(node.fracIndices))) - node.nuRelax(node.non_determined_nu(node.fracIndices)));
               [~,maxind] = max(min([lower_gains, upper_gains],[],2));
               node.BifurcationIndex = node.non_determined_nu(node.fracIndices(maxind));
           else % if no table is available, the constraint on the variance is not active, and only one element should be fractional
%                if length(node.fracIndices) ~= 1
%                    disp('error')
%                end
               node.BifurcationIndex = node.non_determined_nu(node.fracIndices);
           end
           bfI = node.BifurcationIndex;
           bfV = node.nuRelax(node.BifurcationIndex);
           node.BifurcationValueHigh = ceil(bfV);
           node.BifurcationValueLow = floor(bfV);
           % higher branch
           nu_lower_high = node.LowerNuBound;
           nu_lower_high(node.BifurcationIndex) = node.BifurcationValueHigh;
           node.UpperChildNode = NodeMADmax(node.commonParams,nu_lower_high,node.UpperNuBound,node.bestSolution,false,node.depth+1,node);
           % lower branch
           nu_upper_low = node.UpperNuBound;
           nu_upper_low(node.BifurcationIndex) = node.BifurcationValueLow;
           node.LowerChildNode = NodeMADmax(node.commonParams,node.LowerNuBound,nu_upper_low,node.bestSolution,false,node.depth+1,node);
           node.isLeaf = false;
           node.simplexTableau = [];
           node.basicSet = [];
           updateNodeValue(node);
       end
       
       % if values (LowerBound) of child nodes change, they call this update function of
       % their parent node
       % it is also called by the bifurcate method when the child nodes are
       % created
       % updates the lower bound and the open/closed status of the node
       % since the method is called, it is assumed that the child nodes
       % exist
       function updateNodeValue(node)
           old_lower_bound = node.LowerBound;
           old_status = node.nodeOpen;
           if ~node.isLeaf % should be true automatically
               node.LowerBound = min(node.LowerChildNode.LowerBound,node.UpperChildNode.LowerBound);
               % if both child nodes are closed, we may delete them and close the
               % parent node
               % the updated node calls the update routine for the parent node
               if (~node.LowerChildNode.nodeOpen) && (~node.UpperChildNode.nodeOpen)
                   node.nodeOpen = false;
                   delete(node.LowerChildNode);
                   delete(node.UpperChildNode);
                   node.isLeaf = true;
               end
           end
           % ask parent node for update only if something has changed
           if (~node.isRootNode) && ((node.LowerBound ~= old_lower_bound) || (node.nodeOpen ~= old_status))
               updateNodeValue(node.ParentNode);
           end
       end
       
       % is called if the best integer solution has improved
       % this affects all nodes, because their values can become worse than
       % the new upper bound
       % in this case the node is closed and its child nodes can be deleted
       % function is called only for the root node, the call then
       % propagates downwards the whole tree
       function updateByUpperBound(node)
           if node.LowerBound >= node.bestSolution.upperBound
               node.nodeOpen = false;
               if ~node.isLeaf
                   delete(node.LowerChildNode);
                   delete(node.UpperChildNode);
                   node.isLeaf = true;
               else
                   node.simplexTableau = [];
                   node.basicSet = [];
               end
           else
               if ~node.isLeaf
                   updateByUpperBound(node.LowerChildNode);
                   updateByUpperBound(node.UpperChildNode);
               end
           end
       end
       
       % returns the leaf node with the minimal value attached to the input
       % node
       % function operates recursively
       % uses the fact that the value of the parent node is always the
       % minimum of the child node values
       % hence we just descend the minimal branch
       function minNode = findMin(node)
           if node.isLeaf
               minNode = node;
           elseif node.LowerChildNode.LowerBound < node.UpperChildNode.LowerBound
               minNode = findMin(node.LowerChildNode);
           else
               minNode = findMin(node.UpperChildNode);
           end
       end
           
   end
   
end