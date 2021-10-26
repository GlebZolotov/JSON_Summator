function [s,Tab,Bas,NonBas] = quick_return_to_feasibility_short(delta,j,q,Tab,Bas,NonBas)
% function [s,Tab,Bas,NonBas] = quick_return_to_feasibility_short(delta,j,q,Tab,Bas,NonBas)
%
% in the linear system corresponding to the simplex table Tab the basic
% variable j, corresponding to row q, has been changed by delta
% this can be corrected by adjusting the right-hand side of equation q
% if the table becomes infeasible, then a phase 1 like optimization is
% performed to return slack j to zero
% s is a boolean indicating whether a feasible point has been reached
% Bas, NonBas are the indices of the basic and non-basic variables
Tab(q+1,end) = Tab(q+1,end) + delta;
if Tab(q+1,end) < 0
    Tab(q+1,:) = -Tab(q+1,:);
    % adding row corresponding to auxiliary cost function
    Tab = [-Tab(q+1,:); Tab];
    % solving auxiliary program
    opt = false;
    count_advance = 0;
    while ~opt
        count_advance = count_advance + 1;
        [opt,~,Tab,Bas,NonBas] = advance_tableau_short(Tab,Bas,NonBas,true);
    end
    %fprintf("Quick phase 1 from parent table: %d simplex iterations\n",count_advance);
    if Tab(1,end) < -10^(-12)
        s = false;
        Tab = [];
        Bas = [];
        NonBas = [];
    else
        s = true;
        Tab(1,:) = [];
        q = find(Bas == j,1);
        if ~isempty(q) % variable j is still basic
            Tab(q+1,:) = -Tab(q+1,:);
        else % variable j is non-basic
            q = find(NonBas == j,1);
            Tab(:,q) = -Tab(:,q);
        end
    end
else
    s = true;
end