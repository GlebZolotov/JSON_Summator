function [s,Tab,Bas] = return_table_to_feasibility(delta_slack,Tab,Bas)
% function [s,Tab,Bas] = return_table_to_feasibility(delta_slack,Tab,Bas)
%
% adapts a simplex table to a change in the right-hand side of a linear
% equation system with m equations and n variables
% Tab in R^((m+1) x (n+1)) is the table to be corrected, Bas in R^m the basic
% set
% delta_slack in R^n is the column vector of additive changes in the variables
% negative entries mean the corresponding constraints are tightened,
% positive entries mean the constraints are relaxed
% s is a boolean indicating whether a feasible point has been found with the new
% bounds
% we introduce two auxiliary variables y,z >= 0, y+z = 1
% at the current point y = 0, z = 1, z added to basic set
% at the sought feasible point y = 1, z = 0

dump_mat = 0;
if (dump_mat)
    fname='dump/return_table_to_feasibility.mat';
    count = mat_count(fname);

    mat_save(fname, 'in_delta_slack', delta_slack, count);
    mat_save(fname, 'in_Tab', Tab, count);
    mat_save(fname, 'in_Bas', Bas, count);
end

n = length(delta_slack);
Tab = [zeros(1,n), -1, 0, -1; Tab(:,1:n), -Tab(:,1:n)*delta_slack, zeros(size(Tab,1),1), Tab(:,end); zeros(1,n), ones(1,3)];
Bas = [Bas, n+2];
opt = false;
count_advance = 0;
start = tic();
while ~opt
    count_advance = count_advance + 1;
    [opt,~,Tab,Bas] = advance_tableau(Tab,Bas,true);
end
opt_time = toc(start) * 1000.0;
fprintf("TR Phase 1 from parent table: %d simplex iterations\n",count_advance);
fprintf("in %.2f ms.; %.4f ms. per iteration\n", opt_time, opt_time/count_advance);
qz = find(Bas == n+2, 1);
if ~isempty(qz) && (Tab(1,end) > -10^(-12))
    % z variable is still basic, but reached zero
    % find a non-basic variable which can be made basic and replace z
    % the corresponding element in the row qz has to be positive
    non_bas = 1:n+2;
    non_bas(Bas) = [];
    [~,jind] = min(Tab(qz+2,non_bas)); % max -> min, fixed by Roland
    j = non_bas(jind);
    Tab(qz+2,:) = Tab(qz+2,:)/Tab(qz+2,j);
    row_ind = 1:size(Tab,1);
    row_ind(qz+2) = [];
    Tab(row_ind,:) = Tab(row_ind,:) - Tab(row_ind,j)*Tab(qz+2,:);
    Bas(qz) = j;
end
if isempty(qz)
    % auxiliary z variable is non-basic and hence zero, feasible point reached
    qy = find(Bas == n+1, 1); % y must now be basic
    Tab([1, qy+2],:) = [];
    Bas(qy) = [];
    Tab(:,n+(1:2)) = [];
    s = true;
else % z remained positive, there is no feasible point
    s = false;
    Tab = [];
    Bas = [];
end

if (dump_mat)
    mat_save(fname, 'out_s', double(s), count);
    mat_save(fname, 'out_Tab', Tab, count);
    mat_save(fname, 'out_Bas', Bas, count);
end
