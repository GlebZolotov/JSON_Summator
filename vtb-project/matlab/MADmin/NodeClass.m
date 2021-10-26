classdef NodeClass < handle
% class definition for MAD min problem

   properties

      LowerBound double
      LowerChildNode NodeClass
      UpperChildNode NodeClass
      ParentNode NodeClass
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
      % slack(Gr*nu <= 1), slack(rbar*nu >= mu0), a, b
      non_determined_nu uint16 % index set of nu_i which are not fixed by lower and upper bounds

   end

   methods
        function mat_save(node, fname, count)
            mat_save_p(node, fname, count, "");
        end
        
        function mat_save_p(node, fname, count, prefix)
            mat_save_p(fname, 'LowerBound', node.LowerBound, count, prefix);
            mat_save_p(fname, 'LowerNuBound', double(node.LowerNuBound), count, prefix);
            mat_save_p(fname, 'UpperNuBound', double(node.UpperNuBound), count, prefix);
            mat_save_p(fname, 'BifurcationIndex', node.BifurcationIndex, count, prefix);
            mat_save_p(fname, 'BifurcationValueLow', node.BifurcationValueLow, count, prefix);
            mat_save_p(fname, 'BifurcationValueHigh', node.BifurcationValueHigh, count, prefix);
            mat_save_p(fname, 'nodeOpen', double(node.nodeOpen), count, prefix);
            mat_save_p(fname, 'isRootNode', double(node.isRootNode), count, prefix);
            mat_save_p(fname, 'isLeaf', double(node.isLeaf), count, prefix);
            mat_save_p(fname, 'nuRelax', node.nuRelax, count, prefix);
            mat_save_p(fname, 'roundValue', node.roundValue, count, prefix);
            mat_save_p(fname, 'depth', node.depth, count, prefix);
            mat_save_p(fname, 'simplexTableau', node.simplexTableau, count, prefix);
            mat_save_p(fname, 'basicSet', node.basicSet, count, prefix);
            mat_save_p(fname, 'non_determined_nu', node.non_determined_nu, count, prefix);

            % LowerChildNode NodeClass
            % UpperChildNode NodeClass
            % ParentNode NodeClass

            % bestSolution BestIntegerSolution
            % commonParams CommonParameters
            node.commonParams.mat_save_p(fname, count, prefix);
            node.bestSolution.mat_save_p(fname, count, prefix);
        end

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
       function obj = NodeClass(commonParameters,nu_lower,nu_upper,best_solution,is_root_node,nodeDepth,parentNode)
            dump_mat = 0; % 0-off, 1-on, 2-debug
            if (dump_mat > 0)
                fname='dump/node_constructor.mat';
                count = mat_count(fname);

                % FIXME
                obj.BifurcationIndex = 1;
                obj.BifurcationValueLow = 0;
                obj.BifurcationValueHigh = 0;
                obj.LowerBound = 0;
                obj.nodeOpen = false;
                obj.roundValue = 0;

                commonParameters.mat_save(fname, count);
                best_solution.mat_save(fname, count);
                mat_save(fname, 'nu_lower', double(nu_lower), count);
                mat_save(fname, 'nu_upper', double(nu_upper), count);
                mat_save(fname, 'is_root_node', double(is_root_node), count);
                mat_save(fname, 'nodeDepth', uint16(nodeDepth), count);

                if (~is_root_node)
                    parentNode.mat_save_p(fname, count, 'parentNode');
                end
            end

           obj.bestSolution = best_solution;
           obj.commonParams = commonParameters;
           obj.isRootNode = is_root_node;
           obj.depth = nodeDepth;
           if ~is_root_node
               obj.ParentNode = parentNode;
               [nu_lower,nu_upper] = tighten_boundsMADmin(commonParameters,nu_lower,nu_upper);
               if min(double(nu_upper) - double(nu_lower)) < 0 % box for integer variables is empty
                   obj.LowerBound = +Inf;
                   obj.nodeOpen = false;
                   obj.roundValue = +Inf;
                   return;
               elseif sum(abs(double(nu_upper) - double(nu_lower))) == 0 % only one integer point is potentially feasible
                   nu = double(nu_lower);
                   wGr = commonParameters.Gr'*nu;
                   if (commonParameters.mu0 <= commonParameters.rbar*nu) && (commonParameters.wmin <= wGr) && (wGr <= 1) % point feasible
                       value = computeMADvariance(commonParameters,nu);
                       obj.nodeOpen = false;
                       obj.roundValue = value;
                       obj.LowerBound = value;
                       setValues(best_solution,value,nu);
                       return;
                   end
               end
               obj.non_determined_nu = parentNode.non_determined_nu; % this will be updated later by the buildSimplexTable routine or when updating the parent node table
           else
               obj.non_determined_nu = 1:commonParameters.numshares;
           end
           obj.LowerNuBound = nu_lower;
           obj.UpperNuBound = nu_upper;
           n = length(obj.non_determined_nu);

           if (dump_mat > 1) % debug
               obj.mat_save_p(fname, count, 'node1');
           end

           % build simplex table for the LP relaxation
           % either use the table from the parent node or construct one
           % from scratch using the solution of the feasibility check
           if ~is_root_node % if there is a parent node it must have its own table
               Tab = obj.ParentNode.simplexTableau;
               Bas = obj.ParentNode.basicSet;
               % update the slack values according to changed upper and
               % lower bounds
               % differences with bounds of parent node
               delta_upper = double(nu_upper(obj.non_determined_nu)) - double(obj.ParentNode.UpperNuBound(obj.non_determined_nu)); % <= 0
               delta_lower = double(obj.ParentNode.LowerNuBound(obj.non_determined_nu)) - double(nu_lower(obj.non_determined_nu)); % <= 0
               delta_rhs = [delta_lower; delta_upper; zeros(3+2*commonParameters.T,1)];
               if sum(sign(delta_rhs)) == -1 % only one equality constraint has been tightened
                   j = find(delta_rhs < 0, 1);
                   q = find(Bas == j, 1);
                   [s,Tab,Bas] = quick_return_to_feasibility(delta_rhs(j),j,q,Tab,Bas);
                   if ~s % no feasible point
                       obj.LowerBound = +Inf;
                       obj.nodeOpen = false;
                       obj.roundValue = +Inf;
                       return;
                   end
               else % several equality constraints have been tightened
                   [s,Tab,Bas] = return_table_to_feasibility(delta_rhs,Tab,Bas);
                   if ~s % no feasible point
                       obj.LowerBound = +Inf;
                       obj.nodeOpen = false;
                       obj.roundValue = +Inf;
                       return;
                   end
               end

                if (dump_mat > 1) % debug
                   obj.simplexTableau = Tab;
                   obj.basicSet = Bas;
                   obj.mat_save_p(fname, count, 'node1a');
                end

               % we now can eliminate further rows and columns corresponding to equal
               % upper and lower bounds
               ind_equal = flipud(find(nu_upper(obj.non_determined_nu) == nu_lower(obj.non_determined_nu))); % sort in descending order
               if ~isempty(ind_equal)
                   for k = ind_equal'
                       q = find(Bas == n + k, 1);
                       if ~isempty(q)
                           Tab(q+1,:) = [];
                           Bas(q) = [];
                       end
                       Tab(:,n + k) = [];
                       Bas(Bas > n + k) = Bas(Bas > n + k) - 1;
                       q = find(Bas == k, 1);
                       if ~isempty(q)
                           Tab(q+1,:) = [];
                           Bas(q) = [];
                       end
                       Tab(:,k) = [];
                       Bas(Bas > k) = Bas(Bas > k) - 1;
                       n = n - 1;
                   end
                   obj.non_determined_nu(ind_equal) = [];
               end
           else
               [s,Tab,Bas] = buildSimplexTable(obj);
               if ~s
                   obj.LowerBound = +Inf;
                   obj.nodeOpen = false;
                   obj.roundValue = +Inf;
                   return;
               end
           end

           if (dump_mat > 1) % debug
               p_info(Tab);
               p_info(Bas);
               obj.simplexTableau = Tab;
               obj.basicSet = Bas;
               obj.mat_save_p(fname, count, 'node2');
           end

           % solve LP relaxation
           opt = false;
           count_advance = 0;
           while ~opt
               count_advance = count_advance + 1;
               [opt,~,Tab,Bas] = advance_tableau(Tab,Bas,false);
           end
           fprintf("Solution phase: %d simplex iterations\n",count_advance);
           value = -Tab(1,end);
           obj.LowerBound = value;

           if (dump_mat > 1) % debug
               obj.simplexTableau = Tab;
               obj.basicSet = Bas;
               obj.mat_save_p(fname, count, 'node3');
               fprintf("value = %e\n", value);
           end

           % checks if node has higher value than the current best integer one
           if best_solution.upperBound <= value % node gives worse values than the best integer solution found so far
               obj.nodeOpen = false;
               obj.roundValue = +Inf;
           else
               Tab(2:end,end) = max(0,Tab(2:end,end)); % correct small negative values of the basic variables
               obj.simplexTableau = Tab;
               obj.basicSet = Bas;

               if (dump_mat > 1) % debug
                   obj.simplexTableau = Tab;
                   obj.basicSet = Bas;
                   obj.mat_save_p(fname, count, 'node4');
               end

               % restore solution nu from slacks in the table
               nu = double(nu_lower);
               ind_frac = zeros(1,n); % index set of elements in the interior of (l_i,u_i)
               count_frac = 0;
               for i = 1:n
                   q = find(Bas == i, 1);
                   if ~isempty(q) % nu_i is not at lower bound
                       q1 = find(Bas == n+i, 1);
                       if isempty(q1) % nu_i is at upper bound
                           nu(obj.non_determined_nu(i)) = double(nu_upper(obj.non_determined_nu(i)));
                       else % nu_i is neither at lower nor at upper bound
                           nu(obj.non_determined_nu(i)) = nu(obj.non_determined_nu(i)) + Tab(q+1,end);
                           if abs(Tab(q+1,end) - round(Tab(q+1,end))) < 10^(-9) % by chance the intermediate value is also integer
                               nu(obj.non_determined_nu(i)) = round(nu(obj.non_determined_nu(i)));
                           else
                               count_frac = count_frac + 1;
                               ind_frac(count_frac) = obj.non_determined_nu(i);
                           end
                       end
                   end
               end
               ind_frac = ind_frac(1:count_frac);
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
                   [s,~,nu_low,nu_upp] = maximize_return_on_cube(commonParameters,uint16(floor(nu)),uint16(ceil(nu)));
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

           if (dump_mat > 0)
%                obj.simplexTableau = Tab;
%                obj.basicSet = Bas;
               obj.mat_save_p(fname, count, 'node_last');
%                if (count >= 10)
%                     error('node_last');
%                end
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
            dump_mat = 0; % 0-off, 1-on, 2-debug
            if (dump_mat > 0)
                fname='dump/node_bifurcate.mat';
                count = mat_count(fname);

                node.mat_save_p(fname, count, 'node_start');
            end

           num_integer_values = length(find(node.nuRelax == round(node.nuRelax)));
           % higher branch
           nu_lower_high = node.LowerNuBound;
           nu_lower_high(node.BifurcationIndex) = node.BifurcationValueHigh;
           node.UpperChildNode = NodeClass(node.commonParams,nu_lower_high,node.UpperNuBound,node.bestSolution,false,node.depth+1,node);
           % lower branch
           nu_upper_low = node.UpperNuBound;
           nu_upper_low(node.BifurcationIndex) = node.BifurcationValueLow;
           node.LowerChildNode = NodeClass(node.commonParams,node.LowerNuBound,nu_upper_low,node.bestSolution,false,node.depth+1,node);
           node.isLeaf = false;
           updateNodeValue(node);

            if (dump_mat > 0)
                node.mat_save_p(fname, count, 'node_end');
                mat_save(fname, 'num_integer_values', num_integer_values, count);

                if (~node.isLeaf)
                    node.UpperChildNode.mat_save_p(fname, count, 'node_UpperChildNode');
                    node.LowerChildNode.mat_save_p(fname, count, 'node_LowerChildNode');
                end

                if (~node.isRootNode)
                    node.ParentNode.mat_save_p(fname, count, 'node_ParentNode');
                end

%                 if (count >= 200)
%                     error('num_integer_values');
%                 end
            end
       end

       % if values (LowerBound) of child nodes change, they call this update function of
       % their parent node
       % it is also called by the bifurcate method when the child nodes are
       % created
       % updates the lower bound and the open/closed status of the node
       % since the method is called, it is assumed that the child nodes
       % exist
       function updateNodeValue(node)
            dump_mat = 0; % 0-off, 1-on, 2-debug
            if (dump_mat > 0)
                fname='dump/node_updateNodeValue.mat';
                count = mat_count(fname);

                node.mat_save_p(fname, count, 'node_start');
            end

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
       % initializes also the non_determined_nu index list correctly
       % this function is to be used if the simplex table in the parent node is not
       % appropriate or absent
       function [s,Tab,Bas] = buildSimplexTable(node)
           dump_mat = 0; % 0-off, 1-on, 2-debug
           if (dump_mat > 0)
                fname='dump/node_buildSimplexTable.mat';
                count = mat_count(fname);
                node.commonParams.mat_save(fname, count);

                mat_save(fname, 'LowerNuBound', double(node.LowerNuBound), count);
                mat_save(fname, 'UpperNuBound', double(node.UpperNuBound), count);
           end

           [s,value,~,~,nu,~,min_max,index1] = maximize_return_on_cube(node.commonParams,node.LowerNuBound,node.UpperNuBound);
           if ~s || (value < node.commonParams.mu0)
               s = false;
               Tab = [];
               Bas = [];

               if (dump_mat > 0)
                   mat_save(fname, 's', double(s), count);
                   mat_save(fname, 'Tab', Tab, count);
                   mat_save(fname, 'Bas', Bas, count);
               end

               return;
           end
           s = true;
           node.non_determined_nu = find(node.LowerNuBound ~= node.UpperNuBound);
           n = length(node.non_determined_nu);
           determined_nu = 1:node.commonParams.numshares;
           determined_nu(node.non_determined_nu) = []; % indices of nu_i such that l_i = nu_i = u_i

           if (dump_mat > 1)
               mat_save(fname, 'determined_nu', determined_nu, count);
               mat_save(fname, 'non_determined_nu', node.non_determined_nu, count);
           end

           Tab = node.commonParams.zeroTableau;
           Tab(1+(1:node.commonParams.numshares),end) = double(node.LowerNuBound);
           Tab(node.commonParams.numshares+1+(1:node.commonParams.numshares),end) = double(node.UpperNuBound);
           % eliminate nu_i and the corresponding slacks from the table
           Tab(:,end) = Tab(:,end) - Tab(:,determined_nu)*nu(determined_nu);
           Tab(:,[determined_nu, node.commonParams.numshares + determined_nu, 2*node.commonParams.numshares + determined_nu]) = [];
           Tab([1 + determined_nu, 1 + node.commonParams.numshares + determined_nu],:) = [];
           % compute basic index set from solution
           Mnu = node.commonParams.M*nu;
           pos_a = uint16(find(Mnu > 0)'); % positive elements of a
           ind_b = uint16(1:node.commonParams.T);
           ind_b(pos_a) = []; % potentially positive elements of b
           ind_sab = [3*(n+1), 3*(n+1)+pos_a, 3*(n+1)+node.commonParams.T+ind_b]; % these are basic independent of the situation with the wmin and mu0 constraints

           if (dump_mat > 1)
               mat_save(fname, 'pos_a', pos_a, count);
               mat_save(fname, 'pos_b', ind_b, count);
               mat_save(fname, 'ind_sab', ind_sab, count);
           end

           switch min_max
               case 0 % both constraints on Gr'*nu are not active and the corresponding slacks are basic
                   % whether nu_i = l_i or u_i depends on the sign of
                   % rbar_i
                   rbar_pos = find(node.commonParams.rbar(node.non_determined_nu) > 0);
                   rbar_nonpos = 1:n;
                   rbar_nonpos(rbar_pos) = [];
                   Bas = [1:n, n+rbar_pos, 2*n+rbar_nonpos, 3*n+(1:2), ind_sab];
               otherwise % one of the constraints on Gr'*nu is active (depending on min_max = -1 or +1) and one of the slacks is basic
                   kk = (3-min_max)/2;
                   [~,low_ind] = intersect(node.non_determined_nu,node.commonParams.ind_rho_div_Gr(1:index1));
                   [~,upp_ind] = intersect(node.non_determined_nu,node.commonParams.ind_rho_div_Gr(index1:node.commonParams.numshares));
                   Bas = [1:n, n+low_ind', 2*n+upp_ind', 3*n+kk, ind_sab];

                   if (dump_mat > 1)
                       mat_save(fname, 'low_ind', uint16(low_ind), count);
                       mat_save(fname, 'upp_ind', uint16(upp_ind), count);
                   end
           end

           % creating simplex table in normal form
%            Tab = [[1; zeros(2*n+3+node.commonParams.T,1)], Tab(:,Bas)]\Tab;
           A = [[1; zeros(2*n+3+node.commonParams.T,1)], Tab(:,Bas)];
           B = Tab;
           Tab = A\B;
           if (dump_mat > 1)
               mat_save(fname, 'A', A, count);
               mat_save(fname, 'B', B, count);
           end

            % FIXME - dump A,B,Tab for C++ tests
%            zz = A * Tab - B;
%            fprintf("zz_mat: min = %e max = %e\n", min(zz,[],'all'), max(zz,[],'all'));
%            fprintf("zz_mat: nrm1 = %e nrm2 = %e\n", norm(zz, 1), norm(zz));
%            error('Ax=B');

           % removing rows and columns corresponding to nu
           Tab(:,1:n) = [];
           Bas(1:n) = []; % the variables nu are the first n basic columns
           Bas = Bas - n;
           Tab(1+(1:n),:) = [];

           if (dump_mat > 0)
               mat_save(fname, 's', double(s), count);
               mat_save(fname, 'Bas', Bas, count);
               mat_save(fname, 'Tab', Tab, count);
           end
       end
   end
end
