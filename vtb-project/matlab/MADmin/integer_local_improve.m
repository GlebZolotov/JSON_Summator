function [s,value,nu] = integer_local_improve(commonParameters,nu)
% function [s,value,nu] = integer_local_improve(commonParameters,nu)
%
% part of the MAD min problem
% nu is an integer vector
% function checks its feasibility
% if it is feasible, greedy local descent is tried by changing entries by
% +-1
% if the solution cannot be further improved, the corresponding variables
% are given back
% value is the minimal achieved objective value
% nu are the solution variables
% s is a boolean indicating whether a feasible solution has been found
% check feasibility

dump_mat = 0;
if (dump_mat)
    fname='dump/integer_local_improve.mat';
    count = mat_count(fname);

    mat_save(fname, 'in_rbar', commonParameters.rbar, count);
    mat_save(fname, 'in_Gr', commonParameters.Gr, count);
    mat_save(fname, 'in_M', commonParameters.M, count);
    mat_save(fname, 'in_numshares', commonParameters.numshares, count);
    mat_save(fname, 'in_numax', commonParameters.numax, count);
    mat_save(fname, 'in_mu0', commonParameters.mu0, count);
    mat_save(fname, 'in_wmin', commonParameters.wmin, count);    
    mat_save(fname, 'in_nu', double(nu), count);
end

nu = double(nu);
muslack = commonParameters.rbar*nu - commonParameters.mu0;
upperwslack = 1 - nu'*commonParameters.Gr;
lowerwslack = nu'*commonParameters.Gr - commonParameters.wmin;
if min([muslack,upperwslack,lowerwslack]) < 0
    s = false;
    value = [];
    a = [];
    b = [];

    if (dump_mat)
        mat_save(fname, 'out_s', double(s), count);
        mat_save(fname, 'out_value', 0.0, count);
        mat_save(fname, 'out_nu', double(nu), count);
    end

    return;
else
    s = true;
end
% start local descent
ab = commonParameters.M*nu;
value = sum(abs(ab))/2;
% there are 2n candidate steps, (+delta(i),-delta(i))
J = [eye(commonParameters.numshares), -eye(commonParameters.numshares)];
while true
    % find variations not violating feasibility
    feas_ind = find(([nu'+1-double(commonParameters.numax)',1-nu'] <= 0) & ([-commonParameters.rbar,commonParameters.rbar] <= muslack) & ([commonParameters.Gr',-commonParameters.Gr'] <= upperwslack) & ([-commonParameters.Gr',commonParameters.Gr'] <= lowerwslack));
    if isempty(feas_ind)
        break;
    end
    nucandidates = nu*ones(1,length(feas_ind)) + J(:,feas_ind);
    L1vec = ab*ones(1,length(feas_ind)) + commonParameters.M*J(:,feas_ind);
    [mi,minind] = min(sum(abs(L1vec))/2);
    if mi < value
        value = mi;
        ind = feas_ind(minind);
        nu = nucandidates(:,minind);
        ab = L1vec(:,minind);
        if ind <= commonParameters.numshares
            muslack = muslack + commonParameters.rbar(ind);
            upperwslack = upperwslack - commonParameters.Gr(ind);
            lowerwslack = lowerwslack + commonParameters.Gr(ind);
        else
            muslack = muslack - commonParameters.rbar(ind-double(commonParameters.numshares));
            upperwslack = upperwslack + commonParameters.Gr(ind-double(commonParameters.numshares));
            lowerwslack = lowerwslack - commonParameters.Gr(ind-double(commonParameters.numshares));
        end
    else
        break;
    end
end
%a = max(ab,zeros(commonParameters.T,1));
%b = -min(ab,zeros(commonParameters.T,1));
nu = uint16(nu);

if (dump_mat)
    mat_save(fname, 'out_s', double(s), count);
    mat_save(fname, 'out_value', value, count);
    mat_save(fname, 'out_nu', double(nu), count);
end
