function s = return_table_to_feasibility(node,delta_slack)
% function s = return_table_to_feasibility(node,delta_slack)
%
% adapts a simplex table to a change in the right-hand side of a linear
% equation system with m equations and n variables
% node.simplexTableau in R^((m+1) x (n+1)) is the table to be corrected, node.basicSet in R^m the basic
% set
% delta_slack in R^n is the column vector of additive changes in the variables
% negative entries mean the corresponding constraints are tightened,
% positive entries mean the constraints are relaxed
% s is a boolean indicating whether a feasible point has been found with the new
% bounds
% we introduce two auxiliary variables y,z >= 0, y+z = 1
% at the current point y = 0, z = 1, z added to basic set
% at the sought feasible point y = 1, z = 0
n = length(delta_slack);
node.simplexTableau = [zeros(1,n), -1, 0, -1; node.simplexTableau(:,1:n), -node.simplexTableau(:,1:n)*delta_slack, zeros(size(node.simplexTableau,1),1), node.simplexTableau(:,end); zeros(1,n), ones(1,3)];
node.basicSet = [node.basicSet, n+2];
opt = false;
count_advance = 0;
while ~opt
    count_advance = count_advance + 1;
    [opt,~] = advance_tableau(node,true);
end
%fprintf("Phase 1 from parent table: %d simplex iterations\n",count_advance);
qz = find(node.basicSet == n+2, 1);
if ~isempty(qz) && (node.simplexTableau(1,end) > -10^(-12))
    % z variable is still basic, but reached zero
    % find a non-basic variable which can be made basic and replace z
    % the corresponding element in the row qz has to be negative
    non_bas = 1:n+2;
    non_bas(node.basicSet) = [];
    [~,jind] = min(node.simplexTableau(qz+2,non_bas));
    j = non_bas(jind);
    node.simplexTableau(qz+2,:) = node.simplexTableau(qz+2,:)/node.simplexTableau(qz+2,j);
    row_ind = 1:size(node.simplexTableau,1);
    row_ind(qz+2) = [];
    node.simplexTableau(row_ind,:) = node.simplexTableau(row_ind,:) - node.simplexTableau(row_ind,j)*node.simplexTableau(qz+2,:);
    node.basicSet(qz) = j;
    qz = [];
end
if isempty(qz)
    % auxiliary z variable is non-basic and hence zero, feasible point reached
    qy = find(node.basicSet == n+1, 1); % y must now be basic
    node.simplexTableau([1, qy+2],:) = [];
    node.basicSet(qy) = [];
    node.simplexTableau(:,n+(1:2)) = [];
    s = true;
else % z remained positive, there is no feasible point 
    s = false;
    node.simplexTableau = [];
    node.basicSet = [];
end
