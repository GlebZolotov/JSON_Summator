function [s,Tab,Bas,NonBas] = return_table_to_feasibility_short(delta_slack,Tab,Bas,NonBas)
% function [s,Tab,Bas,NonBas] = return_table_to_feasibility_short(delta_slack,Tab,Bas,NonBas)
%
% adapts a simplex table to a change in the right-hand side of a linear
% equation system with m equations and n variables
% Tab in R^((m+1) x (n-m+1)) is the table to be corrected, Bas in R^m the basic
% set, NonBas in R^(n-m) the non-basic set
% delta_slack in R^n is the column vector of additive changes in the variables
% negative entries mean the corresponding constraints are tightened,
% positive entries mean the constraints are relaxed
% s is a boolean indicating whether a feasible point has been found with the new
% bounds
% we introduce two auxiliary variables y,z >= 0, y+z = 1
% at the current point y = 0, z = 1, z added to basic set
% at the sought feasible point y = 1, z = 0
n = length(delta_slack);
m = size(Tab,1) - 1;
%Tab = [zeros(1,n), -1, 0, -1; Tab(:,1:n), -Tab(:,1:n)*delta_slack, zeros(size(Tab,1),1), Tab(:,end); zeros(1,n), ones(1,3)];
Tab = [zeros(1,n-m), -1, -1; Tab(:,1:(n-m)), -Tab(:,1:(n-m))*delta_slack(NonBas)-[0; delta_slack(Bas)], Tab(:,end); zeros(1,n-m), ones(1,2)];
Bas = [Bas, n+2];
NonBas = [NonBas, n+1];
opt = false;
count_advance = 0;
while ~opt
    count_advance = count_advance + 1;
    [opt,~,Tab,Bas,NonBas] = advance_tableau_short(Tab,Bas,NonBas,true);
end
%fprintf("Phase 1 from parent table: %d simplex iterations\n",count_advance);
qz = find(Bas == n+2, 1);
if ~isempty(qz) && (Tab(1,end) > -10^(-12))
    % z variable is still basic, but reached zero
    % find a non-basic variable which can be made basic and replace z
    % the corresponding element in the row qz has to be negative
    [~,j] = min(Tab(qz+2,1:end-1)); % index of entering non-basic column
    indj = NonBas(j); % index of entering non-basic variable
    pivot_element = Tab(qz+2,j);
    Tab(qz+2,j) = 1;
    Tab(qz+2,:) = Tab(qz+2,:)/pivot_element;
    row_ind = 1:size(Tab,1);
    row_ind(qz+2) = [];
    pivot_vector = Tab(row_ind,j);
    Tab(row_ind,j) = 0;
    Tab(row_ind,:) = Tab(row_ind,:) - pivot_vector*Tab(qz+2,:);
    Bas(qz) = indj;
    NonBas(j) = n+2;
    qz = [];
end
if isempty(qz)
    % auxiliary z variable is non-basic and hence zero, feasible point reached
    qy = find(Bas == n+1, 1); % y must now be basic
    Tab([1, qy+2],:) = []; % remove first row and row corresponding to y
    Bas(qy) = [];
    jz = find(NonBas == n+2, 1); % z must now be non-basic
    Tab(:,jz) = []; % remove column corresponding to z
    NonBas(jz) = [];
    s = true;
else % z remained positive, there is no feasible point 
    s = false;
    Tab = [];
    Bas = [];
    NonBas = [];
end
