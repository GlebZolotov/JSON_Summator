function [s,value,nu] = MADmax_integer_local_improve(commonParameters,nu)
% function [s,value,nu] = MADmax_integer_local_improve(commonParameters,nu)
%
% nu is an integer vector
% function checks its feasibility
% if it is feasible, greedy local descent is tried by changing entries by
% +-1
% if it is infeasible only because of the variance constraint, we first try
% to decrease the variance by local descent
% if the solution cannot be further improved, the corresponding variables
% are given back
% value is the maximal achieved objective value
% nu are the solution variables
% s is a boolean indicating whether a feasible solution has been found
% first check feasibility
nu = double(nu);
c = nu'*commonParameters.Gr;
upperwslack = 1 - c;
lowerwslack = c - commonParameters.wmin;
if min(upperwslack,lowerwslack) < 0
    s = false;
    value = [];
    return;
end
old_variance = computeMADvariance(commonParameters,nu);
if old_variance > commonParameters.gammaMAD
    delta_Gr = [commonParameters.Gr; -commonParameters.Gr];
    delta_nu = [eye(length(nu)), -eye(length(nu))];
    while true
        feas_ind = find(([double(commonParameters.numax) - nu; nu] > 0) & (delta_Gr <= upperwslack) & (-delta_Gr <= lowerwslack));
        Mnu = commonParameters.M*nu;
        Mnu_updated = Mnu*ones(1,length(feas_ind)) + commonParameters.M*delta_nu(:,feas_ind);
        variance_updated = mean(abs(Mnu_updated),1);
        [minvar,indminvar] = min(variance_updated);
        if old_variance <= minvar % no improvement in variance
            break;
        else
            old_variance = minvar;
            nu = nu + delta_nu(:,feas_ind(indminvar));
            upperwslack = upperwslack - delta_Gr(feas_ind(indminvar));
            lowerwslack = lowerwslack + delta_Gr(feas_ind(indminvar));
        end
        if old_variance <= commonParameters.gammaMAD % variance constraint satisfied
            break;
        end
    end
end
if old_variance > commonParameters.gammaMAD
    s = false;
    value = [];
    return;
end
s = true;
% start local descent
value = commonParameters.rbar*nu;
% there are n candidate steps, depending on the signs of rbar_i
sign_rbar = sign(commonParameters.rbar);
sign_Gr = (commonParameters.Gr').*sign_rbar;
J = diag(sign_rbar);
while true
    % find variations not violating feasibility
    feas_ind = find((nu'+sign_rbar >= 0) & (nu'+sign_rbar <= double(commonParameters.numax)') & (sign_Gr <= upperwslack) & (-sign_Gr <= lowerwslack));
    for count_feas = length(feas_ind):-1:1
        if computeMADvariance(commonParameters,nu+J(:,feas_ind(count_feas))) > commonParameters.gammaMAD
            feas_ind(count_feas) = [];
        end
    end
    if isempty(feas_ind)
        break; % if we cannot improve the solution we escape the loop
    end
    % choose the index whose change leads to the largest increase in the
    % objective
    [ma,maxind] = max(abs(commonParameters.rbar(feas_ind)));
    value = value + ma;
    ind = feas_ind(maxind);
    nu(ind) = nu(ind) + sign_rbar(ind);
    upperwslack = upperwslack - sign_Gr(ind);
    lowerwslack = lowerwslack + sign_Gr(ind);
end
nu = uint16(nu);
