function [opt,ub] = advance_tableau(node,ph1)
% function [opt,ub] = advance_tableau(node,ph1)
%
% advances valid simplex tableau by one iteration
% ph1 is a boolean indicating whether we are in phase 1 and the second row is a virtual cost
% function; in this case we stop already if the value reaches 10^(-12)
% the simplex tableau and the basic set are taken from the node object
% opt is a boolean indicating whether the current value was at optimality
% ub is a boolean indicating unboundedness
% the output tableau and index set is again written in the node object
[mi,j] = min(node.simplexTableau(1,1:end-1));
if (mi >= -2*10^(-14)) || (ph1 && node.simplexTableau(1,end) > -10^(-12))
    % program is at optimal point
    opt = true;
    ub = false;
    return;
end
opt = false;
% j is the index of the entering basic column
if ph1
    astart = 3;
else
    astart = 2;
end
posind = find(node.simplexTableau(astart:end,j) > 0) + astart - 1;
if isempty(posind)
    % problem unbounded
    ub = true;
    return;
end
ub = false;
baratio = node.simplexTableau(posind,end)./node.simplexTableau(posind,j);
% q is the pivot row
[~,minind] = min(baratio);
q = posind(minind);
% i = node.basicSet(q - astart + 1) is the leaving basic column
node.basicSet(q - astart + 1) = j;
node.simplexTableau(q,:) = node.simplexTableau(q,:)/node.simplexTableau(q,j);
ind = 1:size(node.simplexTableau,1);
ind(q) = [];
node.simplexTableau(ind,:) = node.simplexTableau(ind,:) - node.simplexTableau(ind,j)*node.simplexTableau(q,:);
