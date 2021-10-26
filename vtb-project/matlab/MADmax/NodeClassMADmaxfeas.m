classdef NodeClassMADmaxfeas < handle
% class definition for MAD max feasibility problem
   
   properties
       
      LowerBound double
      LowerChildNode NodeClassMADmaxfeas
      UpperChildNode NodeClassMADmaxfeas
      ParentNode NodeClassMADmaxfeas
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
      basicSet uint16 % basic set of simplex tableau, row k corresponds to variable basicSet(k)
      nonBasicSet uint16 % non-basic set of variables, column k corresponds to variable nonBasicSet(k)
      % the variables corresponding to the columns are:
      % slack (l <= nu), slack(nu <= u), slack(wmin <= Gr*nu),
      % slack(Gr*nu <= 1), a, b
      non_determined_nu uint16 % index set of nu_i which are not fixed by lower and upper bounds
      determined_nu uint16 % index set of nu_i such that l_i = nu_i = u_i
      
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
       function obj = NodeClassMADmaxfeas(commonParameters,nu_lower,nu_upper,best_solution,is_root_node,nodeDepth,parentNode)
           obj.bestSolution = best_solution;
           obj.commonParams = commonParameters;
           obj.isRootNode = is_root_node;
           obj.depth = nodeDepth;
           if ~is_root_node
               obj.ParentNode = parentNode;
               [nu_lower,nu_upper] = tighten_boundsMADmax_feas(commonParameters,nu_lower,nu_upper);
               diff_upper_lower = double(nu_upper) - double(nu_lower);
               if min(diff_upper_lower) < 0 % box for integer variables is empty
                   obj.LowerBound = +Inf;
                   obj.nodeOpen = false;
                   obj.roundValue = +Inf;
                   return;
               elseif sum(abs(diff_upper_lower)) == 0 % only one integer point is potentially feasible
                   nu = double(nu_lower);
                   wGr = commonParameters.Gr'*nu;
                   if (commonParameters.wmin <= wGr) && (wGr <= 1) % point feasible
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
               [s,Tab,Bas,NonBas] = build_simplex_table_from_parent(obj);
           else
               [s,Tab,Bas,NonBas] = build_simplex_table_from_scratch(obj);
           end
           if ~s
               obj.LowerBound = +Inf;
               obj.nodeOpen = false;
               obj.roundValue = +Inf;
               return;
           end
           % solve LP relaxation
           opt = false;
           count_advance = 0;
           while ~opt
               count_advance = count_advance + 1;
               [opt,~,Tab,Bas,NonBas] = advance_tableau_short(Tab,Bas,NonBas,false);
           end
           %time_regular = toc;
           %fprintf("Regular time: %e sec\n",time_regular);
           %fprintf("Solution phase: %d simplex iterations\n",count_advance);
           % save simplex table
           obj.simplexTableau = Tab;
           obj.basicSet = Bas;
           obj.nonBasicSet = NonBas;
           % restore solution nu from slacks in the table
           [nu,ind_frac] = restore_nu_from_table(obj);
           value = -Tab(1,end);
           
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
               if isempty(ind_frac)
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
                   % branching index is determined by highest sensitivity
                   % of the cost function
                   obj.nodeOpen = true;
                   x = commonParameters.M*nu;
                   sMx = sum(diag(sign(x))*commonParameters.M,1);
                   [~,maxind] = max(abs(sMx(ind_frac)));
                   obj.BifurcationIndex = ind_frac(maxind);
                   obj.BifurcationValueLow = floor(nu(obj.BifurcationIndex));
                   obj.BifurcationValueHigh = ceil(nu(obj.BifurcationIndex));
                   % trying to obtain a suboptimal integer solution
                   % first try with round(nu)
                   [s,value,nu_int] = integer_local_improveMADmaxfeas(commonParameters,round(nu));
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
                       [s,value,nu_int] = integer_local_improveMADmaxfeas(commonParameters,nu_low);
                       if s && (value < obj.roundValue)
                           obj.roundValue = value;
                       end
                       if s && (value < best_solution.upperBound)
                           setValues(best_solution,value,nu_int);
                       end
                       [s,value,nu_int] = integer_local_improveMADmaxfeas(commonParameters,nu_upp);
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
       function num_integer_values = bifurcate(node)
           num_integer_values = length(find(node.nuRelax == round(node.nuRelax))); % for recording
           % higher branch
           nu_lower_high = node.LowerNuBound;
           nu_lower_high(node.BifurcationIndex) = node.BifurcationValueHigh;
           node.UpperChildNode = NodeClassMADmaxfeas(node.commonParams,nu_lower_high,node.UpperNuBound,node.bestSolution,false,node.depth+1,node);
           % lower branch
           nu_upper_low = node.UpperNuBound;
           nu_upper_low(node.BifurcationIndex) = node.BifurcationValueLow;
           node.LowerChildNode = NodeClassMADmaxfeas(node.commonParams,node.LowerNuBound,nu_upper_low,node.bestSolution,false,node.depth+1,node);
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
       
       % build a simplex table for the MAD min LP relaxations from a
       % feasible solution nu
       % the feasible solution is obtained by the maximum on cube routine
       % s is a boolean indicating whether the LP is feasible
       % this function is to be used if the simplex table in the parent node is not
       % appropriate or absent (in the current implementation only for the
       % root node)
       function [s,Tab,Bas,NonBas] = build_simplex_table_from_scratch(node)
           [s,~,~,~,nu,~,min_max,index1] = maximize_return_on_cube(node.commonParams,node.LowerNuBound,node.UpperNuBound);
           if ~s % LP relaxation not feasible
               s = false;
               Tab = [];
               Bas = [];
               NonBas = [];
               return;
           end
           n = node.commonParams.numshares;
           T = node.commonParams.T;
           Tab = [zeros(1,3*n+2+T), ones(1,T), 0; ...
                eye(n), -eye(n), zeros(n,n+2+2*T), double(node.LowerNuBound); ...
                eye(n), zeros(n), eye(n), zeros(n,2+2*T), double(node.UpperNuBound); ...
                node.commonParams.Gr', zeros(1,2*n), -1, zeros(1,2*T+1), node.commonParams.wmin; ...
                node.commonParams.Gr', zeros(1,2*n+1), 1, zeros(1,2*T), 1; ...
                node.commonParams.M, zeros(T,2*(n+1)), -eye(T), eye(T), zeros(T,1)];
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
           ind_sab = [3*n+2+pos_a, 3*n+2+node.commonParams.T+ind_b]; % these are basic independent of the situation with the w constraints
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
           Tab = [[1; zeros(2*n+2+node.commonParams.T,1)], Tab(:,Bas)]\Tab;
           % removing rows and columns corresponding to nu(node.non_determined_nu)
           Tab(:,1:n) = [];
           Bas(1:n) = []; % the variables nu(node.non_determined_nu) are the first n basic columns
           Bas = Bas - n;
           Tab(1+(1:n),:) = [];
           % removing basic columns
           NonBas = 1:(size(Tab,2)-1);
           NonBas(Bas) = [];
           Tab(:,Bas) = [];
       end
       
       % build the simplex table for the solution of the LP relaxation for
       % the MAD min problem from the table in the parent node
       function [s,Tab,Bas,NonBas] = build_simplex_table_from_parent(obj)
           Tab = obj.ParentNode.simplexTableau;
           Bas = obj.ParentNode.basicSet;
           NonBas = obj.ParentNode.nonBasicSet;
           n = length(obj.ParentNode.non_determined_nu);
           % update the slack values according to changed upper and
           % lower bounds
           % differences with bounds of parent node
           delta_upper = double(obj.UpperNuBound(obj.ParentNode.non_determined_nu)) - double(obj.ParentNode.UpperNuBound(obj.ParentNode.non_determined_nu)); % <= 0
           delta_lower = double(obj.ParentNode.LowerNuBound(obj.ParentNode.non_determined_nu)) - double(obj.LowerNuBound(obj.ParentNode.non_determined_nu)); % <= 0
           delta_rhs = [delta_lower; delta_upper; zeros(3+2*obj.commonParams.T,1)];
           if sum(sign(delta_rhs)) == -1 % only one equality constraint has been tightened
               j = find(delta_rhs < 0, 1); % variable index, the variable must be basic
               q = find(Bas == j, 1); % row corresponding to the equality
               [s,Tab,Bas,NonBas] = quick_return_to_feasibility_short(delta_rhs(j),j,q,Tab,Bas,NonBas);
           else % several equality constraints have been tightened
               [s,Tab,Bas,NonBas] = return_table_to_feasibility_short(delta_rhs,Tab,Bas,NonBas);
           end
           if ~s % no feasible point
               return;
           end
           % we now can eliminate further rows and columns corresponding to equal
           % upper and lower bounds
           ind_equal = flipud(find(obj.UpperNuBound(obj.ParentNode.non_determined_nu) == obj.LowerNuBound(obj.ParentNode.non_determined_nu)))'; % sort in descending order
           if ~isempty(ind_equal)
               for k = ind_equal
                   q = find(Bas == n + k, 1); % upper slack basic?
                   if ~isempty(q)
                       Tab(q+1,:) = [];
                       Bas(q) = [];
                   else
                       j = find(NonBas == n + k, 1);
                       Tab(:,j) = [];
                       NonBas(j) = [];
                   end
                   Bas(Bas > n + k) = Bas(Bas > n + k) - 1;
                   NonBas(NonBas > n + k) = NonBas(NonBas > n + k) - 1;
                   q = find(Bas == k, 1); % lower slack basic?
                   if ~isempty(q)
                       Tab(q+1,:) = [];
                       Bas(q) = [];
                   else
                       j = find(NonBas == k, 1);
                       Tab(:,j) = [];
                       NonBas(j) = [];
                   end
                   Bas(Bas > k) = Bas(Bas > k) - 1;
                   NonBas(NonBas > k) = NonBas(NonBas > k) - 1;
                   n = n - 1;
               end
           end
       end
       
       % restores nu from the simplex tableau
       % call only if simplex method is used
       function [nu,ind_frac] = restore_nu_from_table(obj)
           nu = double(obj.LowerNuBound);
           n = length(obj.non_determined_nu);
           Tab = obj.simplexTableau;
           Bas = obj.basicSet;
           ind_frac = zeros(1,n); % index set of elements in the interior of (l_i,u_i)
           count_frac = 0;
           for i = 1:n
               q = find(Bas == i, 1);
               if ~isempty(q) % nu_i is not at lower bound
                   q1 = find(Bas == n+i, 1);
                   if isempty(q1) % nu_i is at upper bound
                       nu(obj.non_determined_nu(i)) = double(obj.UpperNuBound(obj.non_determined_nu(i)));
                   else % nu_i is neither at lower nor at upper bound
                       nu(obj.non_determined_nu(i)) = nu(obj.non_determined_nu(i)) + Tab(q+1,end);
                       if Tab(q+1,end) ~= round(Tab(q+1,end)) % by chance the intermediate value may also be integer
                           count_frac = count_frac + 1;
                           ind_frac(count_frac) = obj.non_determined_nu(i);
                       end
                   end
               end
           end
           ind_frac = ind_frac(1:count_frac);
       end
           
   end
   
end