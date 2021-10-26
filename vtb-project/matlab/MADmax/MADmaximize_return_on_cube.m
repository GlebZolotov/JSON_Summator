function [s,single_index,value,nu_low,nu_upp,nu,index] = MADmaximize_return_on_cube(commonParameters,nu_lower,nu_upper,node)
% function [s,single_index,value,nu_low,nu_upp,nu,index] = MADmaximize_return_on_cube(commonParameters,nu_lower,nu_upper,node)
%
% maximizes rbar*nu over the cube {nu: nu_lower <= nu <= nu_upper} under
% the constraints wmin <= nu'*Gr <= 1 and mean(abs(M*nu)) <= gammaMAD
% s is a boolean indicating whether the problem is feasible
% single_index is a boolean indicating whether the upper and lower bounds differ by at most one index (e.g., if the constraint
% mean(abs(M*nu)) <= gammaMAD is not active)
% value is the optimal value
% nu_low,nu_upp are the lower and upper nearest integer vectors to the
% optimal nu
% index is the number of the fractional element (0 if there is none) if
% single_index is true
% node is the node which calls the function
% first we try to solve the problem quickly without the constraint on the
% L1 norm
[s,value,nu_low,nu_upp,nu,index,min_max,index1] = maximize_return_on_cube(commonParameters,nu_lower,nu_upper);
if ~s % problem infeasible
    single_index = [];
    return;
end
if computeMADvariance(commonParameters,nu) <= commonParameters.gammaMAD % a posteriori the variance constraint is satisfied
    single_index = true;
    return;
end
% the constraint on the L1 norm is active and we must take it into account
% and solve the harder LP
% build the simplex tableau: either we use the one from the parent node or
% we build one from the solution of the simplified LP
% if a simplex tableau is available from the parent node, we have to remove
% the rows and columns corresponding to the newly discovered fixed elements
% of nu and to adjust the updated bounds
if ~node.isRootNode && ~isempty(node.ParentNode.simplexTableau)
    % we have a simplex table at the parent node from which to start
    % its LP is only on slacks of elements of nu which are in node.non_determined_nu
    node.simplexTableau = node.ParentNode.simplexTableau;
    node.basicSet = node.ParentNode.basicSet;
    n = length(node.ParentNode.non_determined_nu);
    % differences with bounds of parent node
    delta_upper = double(nu_upper(node.ParentNode.non_determined_nu)) - double(node.ParentNode.UpperNuBound(node.ParentNode.non_determined_nu)); % <= 0
    delta_lower = double(node.ParentNode.LowerNuBound(node.ParentNode.non_determined_nu)) - double(nu_lower(node.ParentNode.non_determined_nu)); % <= 0
    delta_rhs = [delta_lower; delta_upper; zeros(3+2*commonParameters.T,1)];
    if sum(sign(delta_rhs)) == -1 % only one equality constraint is tightened
        j = find(delta_rhs < 0, 1);
        q = find(node.basicSet == j, 1);
        s = quick_return_to_feasibility(node,delta_rhs(j),j,q);
        if ~s % no feasible point
            single_index = [];
            return;
        end
    else % several equality constraints are tightened
        s = return_table_to_feasibility(node,delta_rhs);
        if ~s % no feasible point
            single_index = [];
            return;
        end
    end
    % we now can eliminate further rows and columns corresponding to equal
    % upper and lower bounds
    ind_equal = flipud(find(nu_upper(node.ParentNode.non_determined_nu) == nu_lower(node.ParentNode.non_determined_nu)))'; % sort in descending order
    if ~isempty(ind_equal)
        for k = ind_equal
            q = find(node.basicSet == n + k, 1);
            if ~isempty(q)
                node.simplexTableau(q+1,:) = [];
                node.basicSet(q) = [];
            end
            node.simplexTableau(:,n + k) = [];
            node.basicSet(node.basicSet > n + k) = node.basicSet(node.basicSet > n + k) - 1;
            q = find(node.basicSet == k, 1);
            if ~isempty(q)
                node.simplexTableau(q+1,:) = [];
                node.basicSet(q) = [];
            end
            node.simplexTableau(:,k) = [];
            node.basicSet(node.basicSet > k) = node.basicSet(node.basicSet > k) - 1;
            n = n - 1;
        end
    end
else
    % there is no table in the parent node available
    % we use the unnormalized starting table
    % and adapt it to the solution found by the fast routine
    n = double(commonParameters.numshares);
    Tab = commonParameters.zeroTableau;
    Tab(2+(1:n),end) = double(nu_lower);
    Tab(n+2+(1:n),end) = double(nu_upper);
    n = length(node.non_determined_nu);
    % deleting rows and columns of zero slacks
    ind_det = 1:commonParameters.numshares;
    ind_det(node.non_determined_nu) = []; % indices of determined elements nu_i
    Tab(2+ind_det,:) = []; % delete equations on lower bound slack
    Tab(2+n+ind_det,:) = []; % delete equations on upper bound slack
    Tab(:,end) = Tab(:,end) - Tab(:,ind_det)*double(nu_lower(ind_det)); % eliminate determined nu_i elements: update right-hand side
    Tab(:,ind_det) = []; % delete columns corresponding to nu_i and their lower and upper slacks
    Tab(:,n+ind_det) = [];
    Tab(:,2*n+ind_det) = [];
    % assembling basic index set
    Mnu = commonParameters.M*nu;
    pos_a = uint16(find(Mnu > 0)');
    ind_b = uint16(1:commonParameters.T);
    ind_b(pos_a) = [];
    ind_sab = [3*(n+1), 3*(n+1)+pos_a, 3*(n+1)+commonParameters.T+ind_b];
    switch min_max
        case 0
            rbar_pos = find(commonParameters.rbar(node.non_determined_nu) > 0);
            ind = 1:n;
            ind(rbar_pos) = [];
            rbar_nonpos = ind;
            Bas = [1:n, n+rbar_pos, 2*n+rbar_nonpos, 3*n+(1:2), ind_sab];
        otherwise
            kk = (3-min_max)/2;
            [~,low_ind] = intersect(node.non_determined_nu,commonParameters.ind_rho_div_Gr(1:index1));
            [~,upp_ind] = intersect(node.non_determined_nu,commonParameters.ind_rho_div_Gr(index1:commonParameters.numshares));
            Bas = [1:n, n+low_ind', 2*n+upp_ind', 3*n+kk, ind_sab];
    end
    % creating simplex table in normal form
    Tab = [[eye(2); zeros(2*n+3+commonParameters.T,2)], Tab(:,Bas)]\Tab;
    % removing rows and columns corresponding to nu
    Tab(:,1:n) = [];
    Bas(1:n) = [];
    Bas = Bas - n;
    Tab(2+(1:n),:) = [];
    node.simplexTableau = Tab;
    node.basicSet = Bas;
    % minimizing auxiliary function
    opt = false;
    count_advance = 0;
    while ~opt
        count_advance = count_advance + 1;
        [opt,~] = advance_tableau(node,true);
    end
    %fprintf("Phase 1 from new table: %d simplex iterations\n",count_advance);
    if ~isempty(find(node.basicSet == 2*n+3, 1))
        % feasible set of the real LP is empty
        % slack of variance constraint is basic
        s = false;
        node.simplexTableau = [];
        node.basicSet = [];
        single_index = [];
        return;
    end
    % removing first row corresponding to auxiliary cost
    node.simplexTableau(1,:) = [];
    % column corresponding to the gammaMAD slack is inverted back
    node.simplexTableau(:,2*n+3) = -node.simplexTableau(:,2*n+3);
end
% maximizing return with constraint on variance
opt = false;
count_advance = 0;
while ~opt
    count_advance = count_advance + 1;
    [opt,~] = advance_tableau(node,false);
end
%fprintf("Solution phase: %d simplex iterations\n",count_advance);
% since we maximize the corner element in the table is the value of the cost
value = node.simplexTableau(1,end);
nu = double(nu_lower);
ind_frac = zeros(1,n); % index set of elements in the interior of (l_i,u_i)
count_frac = 0;
for i = 1:n
    q = find(node.basicSet == i, 1);
    if ~isempty(q) % nu_i is not at lower bound
        q1 = find(node.basicSet == n+i, 1);
        if isempty(q1) % nu_i is at upper bound
            nu(node.non_determined_nu(i)) = double(nu_upper(node.non_determined_nu(i)));
        else % nu_i is neither at lower nor at upper bound
            nu(node.non_determined_nu(i)) = nu(node.non_determined_nu(i)) + node.simplexTableau(q+1,end);
            if abs(node.simplexTableau(q+1,end) - round(node.simplexTableau(q+1,end))) < 10^(-9) % by chance the intermediate value is also integer
                nu(node.non_determined_nu(i)) = round(nu(node.non_determined_nu(i)));
            else
                count_frac = count_frac + 1;
                ind_frac(count_frac) = i;
            end
        end
    end
end
ind_frac = ind_frac(1:count_frac);
if isempty(ind_frac)
    % nu is integer
    index = 0;
    single_index = true;
    nu_low = uint16(nu);
    nu_upp = nu_low;
elseif length(ind_frac) == 1
    index = node.non_determined_nu(ind_frac);
    single_index = true;
    nu_low = nu;
    nu_low(node.non_determined_nu(ind_frac)) = floor(nu(node.non_determined_nu(ind_frac)));
    nu_low = uint16(nu_low);
    nu_upp = nu_low;
    nu_upp(node.non_determined_nu(ind_frac)) = uint16(ceil(nu(node.non_determined_nu(ind_frac))));
else
    index = [];
    single_index = false;
    nu_low = nu;
    nu_low(node.non_determined_nu(ind_frac)) = floor(nu(node.non_determined_nu(ind_frac)));
    nu_low = uint16(nu_low);
    nu_upp = nu_low;
    nu_upp(node.non_determined_nu(ind_frac)) = uint16(ceil(nu(node.non_determined_nu(ind_frac))));
end
