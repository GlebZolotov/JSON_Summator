function s = quick_return_to_feasibility(node,delta,j,q)
% function s = quick_return_to_feasibility(node,delta,j,q)
%
% in the linear system corresponding to the simplex table Tab the basic
% variable j, corresponding to row q, has been changed by delta
% this can be corrected by adjusting the right-hand side of equation q
% if the table becomes infeasible, then a phase 1 like optimization is
% performed to return slack j to zero
% s is a boolean indicating whether a feasible point has been reached
node.simplexTableau(q+1,end) = node.simplexTableau(q+1,end) + delta;
if node.simplexTableau(q+1,end) < 0
    node.simplexTableau(q+1,:) = -node.simplexTableau(q+1,:);
    node.simplexTableau(:,j) = -node.simplexTableau(:,j);
    % adding row corresponding to auxiliary cost function
    node.simplexTableau = [-node.simplexTableau(q+1,:); node.simplexTableau];
    node.simplexTableau(1,j) = 0;
    % solving auxiliary program
    opt = false;
    count_advance = 0;
    while ~opt
        count_advance = count_advance + 1;
        [opt,~] = advance_tableau(node,true);
    end
    %fprintf("Phase 1 from parent table: %d simplex iterations\n",count_advance);
    if node.simplexTableau(1,end) < -10^(-12)
        s = false;
        node.simplexTableau = [];
        node.basicSet = [];
    else
        s = true;
        node.simplexTableau(1,:) = [];
        node.simplexTableau(:,j) = -node.simplexTableau(:,j);
        q = find(node.basicSet == j,1);
        if ~isempty(q) % variable j is still basic
            node.simplexTableau(q+1,:) = -node.simplexTableau(q+1,:);
            node.simplexTableau(q+1,end) = 0;
        end
    end
else
    s = true;
end