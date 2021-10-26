function [opt,ub,T,B,NB] = advance_tableau_short(T,B,NB,ph1)
% function [opt,ub,T,B,NB] = advance_tableau_short(T,B,NB,ph1)
%
% advances valid simplex tableau by one iteration
% ph1 is a boolean indicating whether we are in phase 1 and the second row is a virtual cost
% function; in this case we stop already if the objective value reaches 10^(-12)
% T is the current value of the tableau
% B is the current basic index set
% NB is the current non-basic index set
% opt is a boolean indicating whether the current value was at optimality
% ub is a boolean indicating unboundedness
% outputs T, B, NB are the updated objects
[mi,j] = min(T(1,1:end-1));
if (mi >= -10^(-14)) || (ph1 && T(1,end) > -10^(-12))
    % program is at optimal point
    opt = true;
    ub = false;
    return;
end
opt = false;
% j is the index of the non-basic column entering the basic set
indj = NB(j); % index of the entering non-basic variable
if ph1
    astart = 3;
else
    astart = 2;
end
posind = find(T(astart:end,j) > 0) + astart - 1; % positive coefficients in the column
if isempty(posind)
    % problem unbounded
    ub = true;
    return;
end
ub = false;
baratio = T(posind,end)./T(posind,j);
% q is the pivot row
[~,minind] = min(baratio);
q = posind(minind);
i = B(q - astart + 1); % index of the leaving basic variable
B(q - astart + 1) = indj;
NB(j) = i;
pivot_element = T(q,j);
T(q,j) = 1;
T(q,:) = T(q,:)/pivot_element;
ind = 1:size(T,1);
ind(q) = [];
pivot_vector = T(ind,j);
T(ind,j) = 0;
T(ind,:) = T(ind,:) - pivot_vector*T(q,:);
