classdef NodeMADmin < handle
% class definition for MAD min problem
   
   properties
       
      LowerBound double
      LowerChildNode NodeMADmin
      UpperChildNode NodeMADmin
      ParentNode NodeMADmin
      LowerNuBound uint16 % lower bounds on integer variables
      UpperNuBound uint16 % upper bounds on integer variables
      Sensitivities double % n x 2 array containing the sensitivities of the cost with respect to changes of the variable upwards and downwards
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
      basicSet uint16 % basic set of simplex tableau, row k corresponds to variable basicSet(k)
      nonBasicSet uint16 % non-basic set of variables, column k corresponds to variable nonBasicSet(k)
      % the variables corresponding to the columns are:
      % slack (l <= nu), slack(nu <= u), slack(wmin <= Gr*nu),
      % slack(Gr*nu <= 1), slack(rbar*nu >= mu0), a, b
      non_determined_nu uint16 % index set of nu_i which are not fixed by lower and upper bounds
      determined_nu uint16 % index set of nu_i such that l_i = nu_i = u_i
      fracIndices uint16 % index set of fractional nu variables among the non_determined_nu
      lowerBasic uint16 % row indices of lower slacks of fractional variables
      upperBasic uint16 % row indices of upper slacks of fractional variables
      
   end
   
   methods
       
       % CONSTRUCTOR
       % creates a new node
       % if the argument parentNode is empty, this is the root node
       % when it is created, the node solves its LP relaxation and
       % memorizes the corresponding simplex table
       % if the value is above the value of the current best solution, the
       % node declares itself closed
       % if the LP solution is integer, then the node is also closed
       % if the integer solution is better than the current best, it
       % replaces the best solution
       % if the value is lower than the current best and not integer, the
       % node declares itself as open and computes the index of the
       % variable for branching
       function obj = NodeMADmin(commonParameters,nu_lower,nu_upper,best_solution,is_root_node,nodeDepth,parentNode)
           obj.bestSolution = best_solution;
           obj.commonParams = commonParameters;
           obj.isRootNode = is_root_node;
           obj.depth = nodeDepth;
           if ~is_root_node
               obj.ParentNode = parentNode;
               [nu_lower,nu_upper] = tighten_boundsMADmin(commonParameters,nu_lower,nu_upper);
               diff_upper_lower = double(nu_upper) - double(nu_lower);
               if min(diff_upper_lower) < 0 % box for integer variables is empty
                   obj.LowerBound = +Inf;
                   obj.nodeOpen = false;
                   obj.roundValue = +Inf;
                   return;
               elseif sum(abs(diff_upper_lower)) == 0 % only one integer point is potentially feasible
                   nu = double(nu_lower);
                   wGr = commonParameters.Gr'*nu;
                   if (commonParameters.mu0 <= commonParameters.rbar*nu) && (commonParameters.wmin <= wGr) && (wGr <= 1) % point feasible
                       value = computeMADvariance(commonParameters,nu);
                       obj.nodeOpen = false;
                       obj.roundValue = value;
                       obj.LowerBound = value;
                       obj.nuRelax = nu;
                       setValues(best_solution,value,nu);
                       return;
                   else % point not feasible
                       obj.LowerBound = +Inf;
                       obj.nodeOpen = false;
                       obj.roundValue = +Inf;
                       return;
                   end
               end
           end
           obj.LowerNuBound = nu_lower;
           obj.UpperNuBound = nu_upper;
           obj.non_determined_nu = find(nu_lower ~= nu_upper);
           obj.determined_nu = find(nu_lower == nu_upper);
           
           % solve LP relaxation of the node
           
%            tic;
%            [s,value,nu] = cvxsolve(obj);
%            time_cvx = toc;
%            fprintf("cvx time: %e sec\n",time_cvx);
           
%            tic;
%            [s,value,nu,ind_frac] = interior_point_solve(obj);
%            time_ip = toc;
%            fprintf("interior point time: %e sec\n",time_ip);
%            if ~s
%                obj.LowerBound = +Inf;
%                obj.nodeOpen = false;
%                obj.roundValue = +Inf;
%                return;
%            end
           
           % build simplex table for the LP relaxation
           % either use the table from the parent node or construct one
           % from scratch using the solution of the feasibility check
           %tic;
           if ~is_root_node % if there is a parent node it must have its own table
               s = build_simplex_table_from_parent(obj);
           else
               s = build_simplex_table_from_scratch(obj);
           end
           if ~s
               obj.LowerBound = +Inf;
               obj.nodeOpen = false;
               obj.roundValue = +Inf;
               return;
           end
           % restore solution nu from slacks in the table
           nu = restore_nu_from_table(obj);
           value = -obj.simplexTableau(1,end);
           
           obj.LowerBound = value;
           % checks if node has higher value than the current best integer one
           if best_solution.upperBound <= value % node gives worse values than the best integer solution found so far
               obj.nodeOpen = false;
               obj.roundValue = +Inf;
               obj.simplexTableau = [];
               obj.basicSet = [];
               obj.nonBasicSet = [];
           else
               obj.nuRelax = nu;
               % checks if node yields an integer solution
               % look for non-integer value in the solution with the highest
               % sensitivity of the cost function
               if isempty(obj.fracIndices)
                   % obtained integer solution, it is better than the best
                   % solution
                   obj.nodeOpen = false;
                   obj.roundValue = value;
                   setValues(best_solution,value,nu);
                   obj.simplexTableau = [];
                   obj.basicSet = [];
                   obj.nonBasicSet = [];
               else
                   % obtained non-integer solution is better than current
                   % integer value
                   obj.nodeOpen = true;
                   % trying to obtain a suboptimal integer solution
                   % first try with round(nu)
                   [s,value,nu_int] = integer_local_improve(commonParameters,round(nu));
                   if s
                       obj.roundValue = value;
                   else
                       obj.roundValue = +Inf;
                   end
                   if s && (value < best_solution.upperBound)
                       setValues(best_solution,value,nu_int);
                   end
                   % solving auxiliary LP on the unit cube surrounding the
                   % fractional solution and trying to improve locally
                   [s,~,nu_low,nu_upp] = maximize_return_on_cube(commonParameters,floor(nu),ceil(nu));
                   if s
                       [s,value,nu_int] = integer_local_improve(commonParameters,nu_low);
                       if s && (value < obj.roundValue)
                           obj.roundValue = value;
                       end
                       if s && (value < best_solution.upperBound)
                           setValues(best_solution,value,nu_int);
                       end
                       [s,value,nu_int] = integer_local_improve(commonParameters,nu_upp);
                       if s && (value < obj.roundValue)
                           obj.roundValue = value;
                       end
                       if s && (value < best_solution.upperBound)
                           setValues(best_solution,value,nu_int);
                       end
                   end
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
       % uses the previously computes index and values of the branching
       % variable
       % gives back the number of integer values in the vector of integer
       % variables
       function [num_integer_values,bfI,bfV] = bifurcate(node)
           num_integer_values = length(find(node.nuRelax == round(node.nuRelax))); % for recording
           % determine branch variable
           % this is based on the sensitivities of the cost with respect to
           % changes in the fractional variables
           node.Sensitivities = zeros(length(node.fracIndices),2);
           for k = 1:length(node.fracIndices)
               negind = find(node.simplexTableau(node.lowerBasic(k)+1,1:end-1) < 0);
               if isempty(negind)
                   node.Sensitivities(k,1) = Inf;
               else
                   node.Sensitivities(k,1) = min(-node.simplexTableau(1,negind)./node.simplexTableau(node.lowerBasic(k)+1,negind));
               end
               negind = find(node.simplexTableau(node.upperBasic(k)+1,1:end-1) < 0);
               if isempty(negind)
                   node.Sensitivities(k,2) = Inf;
               else
                   node.Sensitivities(k,2) = min(-node.simplexTableau(1,negind)./node.simplexTableau(node.upperBasic(k)+1,negind));
               end
           end
           lower_gains = node.Sensitivities(:,2).*(node.nuRelax(node.non_determined_nu(node.fracIndices)) - floor(node.nuRelax(node.non_determined_nu(node.fracIndices))));
           upper_gains = node.Sensitivities(:,1).*(ceil(node.nuRelax(node.non_determined_nu(node.fracIndices))) - node.nuRelax(node.non_determined_nu(node.fracIndices)));
           [~,maxind] = max(min([lower_gains, upper_gains],[],2));
           node.BifurcationIndex = node.non_determined_nu(node.fracIndices(maxind));
           bfI = node.BifurcationIndex;
           bfV = node.nuRelax(node.BifurcationIndex);
           node.BifurcationValueHigh = ceil(bfV);
           node.BifurcationValueLow = floor(bfV);
           % higher branch
           nu_lower_high = node.LowerNuBound;
           nu_lower_high(node.BifurcationIndex) = node.BifurcationValueHigh;
           node.UpperChildNode = NodeMADmin(node.commonParams,nu_lower_high,node.UpperNuBound,node.bestSolution,false,node.depth+1,node);
           % lower branch
           nu_upper_low = node.UpperNuBound;
           nu_upper_low(node.BifurcationIndex) = node.BifurcationValueLow;
           node.LowerChildNode = NodeMADmin(node.commonParams,node.LowerNuBound,nu_upper_low,node.bestSolution,false,node.depth+1,node);
           node.isLeaf = false;
           node.simplexTableau = [];
           node.basicSet = [];
           node.nonBasicSet = [];
           updateNodeValue(node);
       end
       
       % if values (LowerBound) of child nodes change, they call this update function of
       % their parent node
       % it is initially called by the bifurcate method when the child nodes are
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
       % function is initially called only for the root node, the call then
       % propagates downwards the whole tree
       function updateByUpperBound(node)
           if node.LowerBound >= node.bestSolution.upperBound
               node.nodeOpen = false;
               if node.isLeaf
                   node.simplexTableau = [];
                   node.basicSet = [];
                   node.nonBasicSet = [];
               else
                   delete(node.LowerChildNode);
                   delete(node.UpperChildNode);
                   node.isLeaf = true;
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
       % the minimal of the two child nodes is active, otherwise the child
       % nodes would have been removed already
       function minNode = findMin(node)
           if node.isLeaf
               minNode = node;
           elseif node.LowerChildNode.LowerBound < node.UpperChildNode.LowerBound
               minNode = findMin(node.LowerChildNode);
           else
               minNode = findMin(node.UpperChildNode);
           end
       end
       
       % solve the LP relaxation of the node with an interior-point method
       % this is not called in the current implementation
       function [s,value,nu,ind_frac] = interior_point_solve(node)
           box_midpoint = (double(node.UpperNuBound) + double(node.LowerNuBound))/2;
           box_size = (double(node.UpperNuBound) - double(node.LowerNuBound))/2;
           g = node.commonParams.Gr.*box_size;
           r = node.commonParams.rbar'.*box_size;
           h = node.commonParams.mu0 - node.commonParams.rbar*box_midpoint;
           delta_w = node.commonParams.Gr'*box_midpoint;
           wl = node.commonParams.wmin - delta_w;
           wu = 1 - delta_w;
           b = node.commonParams.M*box_midpoint;
           C = node.commonParams.M*diag(box_size);
           [s,value,x_short] = interior_point_full(b,C(:,node.non_determined_nu),g(node.non_determined_nu),r(node.non_determined_nu),h,wl,wu);
           if s
               x = zeros(node.commonParams.numshares,1);
               x(node.non_determined_nu) = x_short;
               nu = box_midpoint + box_size.*x; % elements equal to upper or lower boud should be exactly integer
               ind_frac = node.non_determined_nu;
               ind_frac(abs(x_short - round(x_short)) < 10^(-9)) = [];
           else
               value = +Inf;
               nu = [];
               ind_frac = [];
           end
       end
       
       % build a simplex table for the MAD min LP relaxations from a
       % feasible solution nu
       % the feasible solution is obtained by the maximum on cube routine
       % s is a boolean indicating whether the LP is feasible
       % this function is to be used if the simplex table in the parent node is not
       % appropriate or absent (in the current implementation only for the
       % root node)
       % the table and the index sets are written in the corresponding
       % fields of the node object
       function s = build_simplex_table_from_scratch(node)
           [s,value,~,~,nu,~,min_max,index1] = maximize_return_on_cube(node.commonParams,node.LowerNuBound,node.UpperNuBound);
           if ~s || (value < node.commonParams.mu0) % LP relaxation not feasible
               s = false;
               node.simplexTableau = [];
               node.basicSet = [];
               node.nonBasicSet = [];
               return;
           end
           Tab = node.commonParams.zeroTableau;
           Tab(1+(1:node.commonParams.numshares),end) = double(node.LowerNuBound);
           Tab(node.commonParams.numshares+1+(1:node.commonParams.numshares),end) = double(node.UpperNuBound);
           % eliminate determined nu_i and the corresponding slacks from the table
           Tab(:,end) = Tab(:,end) - Tab(:,node.determined_nu)*double(node.LowerNuBound(node.determined_nu));
           Tab(:,[node.determined_nu, node.commonParams.numshares + node.determined_nu, 2*node.commonParams.numshares + node.determined_nu]) = [];
           Tab([1 + node.determined_nu, 1 + node.commonParams.numshares + node.determined_nu],:) = [];
           % compute basic index set from solution
           n = length(node.non_determined_nu);
           Mnu = node.commonParams.M*nu;
           pos_a = find(Mnu > 0)'; % positive elements of a
           ind_b = 1:node.commonParams.T;
           ind_b(pos_a) = []; % potentially positive elements of b
           ind_sab = [3*(n+1), 3*(n+1)+pos_a, 3*(n+1)+node.commonParams.T+ind_b]; % these are basic independent of the situation with the wmin and mu0 constraints
           switch min_max
               case 0 % both constraints on Gr'*nu are not active and the corresponding slacks are basic
                   % whether nu_i = l_i or u_i depends on the sign of
                   % rbar_i
                   rbar_pos = find(commonParameters.rbar(node.non_determined_nu) > 0);
                   ind = 1:n;
                   ind(rbar_pos) = [];
                   rbar_nonpos = ind;
                   Bas = [1:n, n+rbar_pos, 2*n+rbar_nonpos, 3*n+(1:2), ind_sab];
               otherwise % one of the constraints on Gr'*nu is active (depending on min_max = -1 or +1) and one of the slacks is basic
                   kk = (3-min_max)/2;
                   [~,low_ind] = intersect(node.non_determined_nu,node.commonParams.ind_rho_div_Gr(1:index1));
                   [~,upp_ind] = intersect(node.non_determined_nu,node.commonParams.ind_rho_div_Gr(index1:node.commonParams.numshares));
                   Bas = [1:n, n+low_ind', 2*n+upp_ind', 3*n+kk, ind_sab];
           end
           % creating simplex table in normal form
           Tab = [[1; zeros(2*n+3+node.commonParams.T,1)], Tab(:,Bas)]\Tab;
           % removing rows and columns corresponding to nu(node.non_determined_nu)
           Tab(:,1:n) = [];
           Bas(1:n) = []; % the variables nu(node.non_determined_nu) are the first n basic columns
           Bas = Bas - n;
           Tab(1+(1:n),:) = [];
           % removing basic columns
           NonBas = 1:(size(Tab,2)-1);
           NonBas(Bas) = [];
           Tab(:,Bas) = [];
           node.simplexTableau = Tab;
           node.basicSet = Bas;
           node.nonBasicSet = NonBas;
           primalSimplex(node,false); % optimizing objective
       end
       
       % build the simplex table for the solution of the LP relaxation for
       % the MAD min problem from the table in the parent node
       % the tableau and the basic and non-basic sets are written in the
       % corresponding fields of the node
       function s = build_simplex_table_from_parent(obj)
           obj.simplexTableau = obj.ParentNode.simplexTableau;
           obj.basicSet = obj.ParentNode.basicSet;
           obj.nonBasicSet = obj.ParentNode.nonBasicSet;
           n = length(obj.ParentNode.non_determined_nu);
           % update the slack values according to changed upper and
           % lower bounds
           % differences with bounds of parent node
           delta_upper = double(obj.UpperNuBound(obj.ParentNode.non_determined_nu)) - double(obj.ParentNode.UpperNuBound(obj.ParentNode.non_determined_nu)); % <= 0
           delta_lower = double(obj.ParentNode.LowerNuBound(obj.ParentNode.non_determined_nu)) - double(obj.LowerNuBound(obj.ParentNode.non_determined_nu)); % <= 0
           delta_rhs = [delta_lower; delta_upper; zeros(3+2*obj.commonParams.T,1)];
           if sum(sign(delta_rhs)) == -1 % only one equality constraint has been tightened, namely that one the branching variable
               j = find(delta_rhs < 0, 1); % variable index, the variable must be basic
               q = find(obj.basicSet == j, 1); % row corresponding to the equality
               obj.simplexTableau(q+1,end) = obj.simplexTableau(q+1,end) + delta_rhs(j); % tableau becomes infeasible but stays dual feasible
               s = dual_simplex(obj); % return to optimal solution with new bound
           else % several equality constraints have been tightened
               s = return_table_to_feasibility(obj,delta_rhs);
           end
           if ~s % no feasible point
               return;
           end
           % we now can eliminate further rows and columns corresponding to equal
           % upper and lower bounds
           ind_equal = flipud(find(obj.UpperNuBound(obj.ParentNode.non_determined_nu) == obj.LowerNuBound(obj.ParentNode.non_determined_nu)))'; % sort in descending order
           if ~isempty(ind_equal)
               for k = ind_equal
                   q = find(obj.basicSet == n + k, 1); % upper slack basic?
                   if ~isempty(q)
                       obj.simplexTableau(q+1,:) = [];
                       obj.basicSet(q) = [];
                   else
                       j = find(obj.nonBasicSet == n + k, 1);
                       obj.simplexTableau(:,j) = [];
                       obj.nonBasicSet(j) = [];
                   end
                   obj.basicSet(obj.basicSet > n + k) = obj.basicSet(obj.basicSet > n + k) - 1;
                   obj.nonBasicSet(obj.nonBasicSet > n + k) = obj.nonBasicSet(obj.nonBasicSet > n + k) - 1;
                   q = find(obj.basicSet == k, 1); % lower slack basic?
                   if ~isempty(q)
                       obj.simplexTableau(q+1,:) = [];
                       obj.basicSet(q) = [];
                   else
                       j = find(obj.nonBasicSet == k, 1);
                       obj.simplexTableau(:,j) = [];
                       obj.nonBasicSet(j) = [];
                   end
                   obj.basicSet(obj.basicSet > k) = obj.basicSet(obj.basicSet > k) - 1;
                   obj.nonBasicSet(obj.nonBasicSet > k) = obj.nonBasicSet(obj.nonBasicSet > k) - 1;
                   n = n - 1;
               end
           end
       end
       
       % returns table in the node object to feasibility and optimizes real
       % objective if feasibility has been reached
       % delta_slack are the changes in the variables
       function s = return_table_to_feasibility(obj,delta_slack)
           % adapts a simplex table to a change in the right-hand side of a linear
           % equation system with m equations and n variables
           % obj.simplexTableau in R^((m+1) x (n-m+1)) is the table to be corrected, obj.basicSet in R^m the basic
           % set, obj.nonBasicSet in R^(n-m) the non-basic set
           % delta_slack in R^n is the column vector of additive changes in the variables
           % negative entries mean the corresponding constraints are tightened,
           % positive entries mean the constraints are relaxed
           % s is a boolean indicating whether a feasible point has been found with the new
           % bounds
           % the function works directly with the table and index sets of
           % the node object
           % we introduce two auxiliary variables y,z >= 0, y+z = 1
           % at the current point y = 0, z = 1, z added to basic set
           % at the sought feasible point y = 1, z = 0
           n = length(delta_slack);
           m = size(obj.simplexTableau,1) - 1;
           obj.simplexTableau = [zeros(1,n-m), -1, -1; obj.simplexTableau(:,1:(n-m)), -obj.simplexTableau(:,1:(n-m))*delta_slack(obj.nonBasicSet)-[0; delta_slack(obj.basicSet)], obj.simplexTableau(:,end); zeros(1,n-m), ones(1,2)];
           obj.basicSet = [obj.basicSet, n+2];
           obj.nonBasicSet = [obj.nonBasicSet, n+1];
           primalSimplex(obj,true);
           %fprintf("Phase 1 from parent table: %d simplex iterations\n",count_advance);
           qz = find(obj.basicSet == n+2, 1);
           if ~isempty(qz) && (obj.simplexTableau(1,end) > -10^(-12))
               % z variable is still basic, but reached zero
               % find a non-basic variable which can be made basic and replace z
               % the corresponding element in the row qz has to be negative
               [~,j] = min(obj.simplexTableau(qz+2,1:end-1)); % index of entering non-basic column
               index_change(obj,qz,j,true);
               qz = [];
           end
           if isempty(qz)
               % auxiliary z variable is non-basic and hence zero, feasible point reached
               qy = find(obj.basicSet == n+1, 1); % y must now be basic
               obj.simplexTableau([1, qy+2],:) = []; % remove first row and row corresponding to y
               obj.basicSet(qy) = [];
               jz = find(obj.nonBasicSet == n+2, 1); % z must now be non-basic
               obj.simplexTableau(:,jz) = []; % remove column corresponding to z
               obj.nonBasicSet(jz) = [];
               primalSimplex(obj,false); % optimize real cost function
               s = true;
           else % z remained positive, there is no feasible point
               s = false;
               obj.simplexTableau = [];
               obj.basicSet = [];
               obj.nonBasicSet = [];
           end
       end
       
       % primal simplex algorithm
       % virtual_row is a boolean indicating whether an auxiliary cost row
       % has been added at the top of the table
       % works directly with the table and the index sets of the table
       % it is assumed that the initial simplex table is feasible and the
       % problem is bounded
       function primalSimplex(obj,virtual_row)
           while true
               [mi,j] = min(obj.simplexTableau(1,1:end-1));
               if (mi >= -10^(-14)) || (virtual_row && obj.simplexTableau(1,end) > -10^(-12))
                   % program is at optimal point
                   break;
               end
               % j is the index of the non-basic column entering the basic set
               if virtual_row
                   astart = 3;
               else
                   astart = 2;
               end
               posind = find(obj.simplexTableau(astart:end,j) > 0) + astart - 1; % positive coefficients in the column
               if isempty(posind)
                   % problem unbounded
                   disp('error');
                   break;
               end
               baratio = obj.simplexTableau(posind,end)./obj.simplexTableau(posind,j);
               % q is the pivot basic variable
               [~,minind] = min(baratio);
               q = posind(minind) - astart + 1;
               index_change(obj,q,j,virtual_row)
           end
       end

       % dual simplex algorithm
       % operates directly on the table and basic and non-basic sets in the
       % node
       % s is a boolean indicating whether the problem is primal feasible
       % or not
       % it is assumed that there is no virtual row at the top
       % and that the top row has only nonnegative entries (except the last
       % element)
       function s = dual_simplex(node)
           while true
               [mi,q] = min(node.simplexTableau(2:end,end));
               % q is the pivot row
               if mi >= -2*10^(-14)
                   % program is at optimal point
                   s = true;
                   break;
               end
               posind = find(node.simplexTableau(q+1,1:end-1) < 0); % negative coefficients in the row
               if isempty(posind)
                   % dual problem unbounded, primal infeasible
                   s = false;
                   break;
               end
               c_ratio = node.simplexTableau(1,posind)./node.simplexTableau(q+1,posind);
               [~,maxind] = max(c_ratio);
               j = posind(maxind); % entering non-basic column
               index_change(node,q,j,false);
           end
       end
       
       % makes one index change between basic and non-basic set in the
       % tableau
       % virtual_row is a boolean indicating whether there is a virtual row
       % added at the top of the tableau
       function index_change(obj,row_var,col_var,virtual_row)
           if virtual_row
               row_index = row_var + 2;
           else
               row_index = row_var + 1;
           end
           a = obj.simplexTableau(row_index,col_var);
           obj.simplexTableau(:,col_var) = -obj.simplexTableau(:,col_var);
           obj.simplexTableau(row_index,col_var) = 1;
           v = obj.simplexTableau(:,col_var);
           h = obj.simplexTableau(row_index,:)/a;
           obj.simplexTableau(:,col_var) = 0;
           obj.simplexTableau(row_index,:) = 0;
           obj.simplexTableau = obj.simplexTableau + v*h;
           q = obj.basicSet(row_var);
           obj.basicSet(row_var) = obj.nonBasicSet(col_var);
           obj.nonBasicSet(col_var) = q;
       end
       
       % restores nu from the simplex tableau
       % call only if simplex method is used
       function nu = restore_nu_from_table(obj)
           nu = double(obj.LowerNuBound);
           n = length(obj.non_determined_nu);
           obj.fracIndices = zeros(1,n); % index set of elements in the interior of (l_i,u_i)
           lower_basic = zeros(1,n);
           upper_basic = zeros(1,n);
           count_frac = 0;
           for i = 1:n
               q = find(obj.basicSet == i, 1);
               if ~isempty(q) % nu_i is not at lower bound
                   q1 = find(obj.basicSet == n+i, 1);
                   if isempty(q1) % nu_i is at upper bound
                       nu(obj.non_determined_nu(i)) = double(obj.UpperNuBound(obj.non_determined_nu(i)));
                   else % nu_i is neither at lower nor at upper bound
                       nu(obj.non_determined_nu(i)) = nu(obj.non_determined_nu(i)) + obj.simplexTableau(q+1,end);
                       if obj.simplexTableau(q+1,end) ~= round(obj.simplexTableau(q+1,end)) % by chance the intermediate value may also be integer
                           count_frac = count_frac + 1;
                           obj.fracIndices(count_frac) = i;
                           obj.lowerBasic(count_frac) = q;
                           obj.upperBasic(count_frac) = q1;
                       end
                   end
               end
           end
           obj.fracIndices = obj.fracIndices(1:count_frac);
       end
           
   end
   
end