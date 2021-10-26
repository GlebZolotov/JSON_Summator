function [opt,ub,T,B] = advance_tableau(T,B,ph1)
% function [opt,ub,T,B] = advance_tableau(T,B,ph1)
%
% advances valid simplex tableau by one iteration
% ph1 is a boolean indicating whether the second row is a virtual cost
% function as in phase 1
% T is the current value of the tableau
% B is the current basic index set
% opt is a boolean indicating whether the current value was at optimality
% ub is a boolean indicating unboundedness
% outputs T and B are the updated objects

dump_mat = false;
if (dump_mat)
    fname='dump/advance_tableau.mat';
    count = mat_count(fname);

    mat_save(fname, 'in_T', T, count);
    mat_save(fname, 'in_B', B, count);
    mat_save(fname, 'in_ph1', double(ph1), count);
end

[mi,j] = min(T(1,1:end-1));
if mi >= -10^(-14)
    % program is at optimal point
    opt = true;
    ub = false;

    if (dump_mat)
        mat_save(fname, 'out_opt', double(opt), count);
        mat_save(fname, 'out_ub', double(ub), count);
        mat_save(fname, 'out_T', T, count);
        mat_save(fname, 'out_B', B, count);
    end

    return;
end
opt = false;
% j is the index of the entering basic column
if ph1
    astart = 3;
else
    astart = 2;
end
posind = find(T(astart:end,j) > 0) + astart - 1;
if isempty(posind)
    % problem unbounded
    ub = true;

    if (write_hdf)
        mat_save(fname, 'out_opt', double(opt), count);
        mat_save(fname, 'out_ub', double(ub), count);
        mat_save(fname, 'out_T', T, count);
        mat_save(fname, 'out_B', B, count);
    end

    return;
end
ub = false;
baratio = T(posind,end)./T(posind,j);
% q is the pivot row
[~,minind] = min(baratio);
q = posind(minind);
% i = B(q - astart + 1) is the leaving basic column
B(q - astart + 1) = j;
T(q,:) = T(q,:)/T(q,j);
ind = 1:size(T,1);
ind(q) = [];
T(ind,:) = T(ind,:) - T(ind,j)*T(q,:);

if (dump_mat)
    mat_save(fname, 'out_opt', double(opt), count);
    mat_save(fname, 'out_ub', double(ub), count);
    mat_save(fname, 'out_T', T, count);
    mat_save(fname, 'out_B', B, count);
end
