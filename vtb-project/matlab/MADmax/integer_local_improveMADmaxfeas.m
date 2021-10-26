function [s,value,nu] = integer_local_improveMADmaxfeas(commonParameters,nu)
% function [s,value,nu] = integer_local_improveMADmaxfeas(commonParameters,nu)
%
% part of the MAD max feasibility check
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
nu = double(nu);
upperwslack = 1 - nu'*commonParameters.Gr;
lowerwslack = nu'*commonParameters.Gr - commonParameters.wmin;
if min(upperwslack,lowerwslack) < 0
    s = false;
    value = [];
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
    feas_ind = find(([nu'+1-double(commonParameters.numax)',1-nu'] <= 0) & ([commonParameters.Gr',-commonParameters.Gr'] <= upperwslack) & ([-commonParameters.Gr',commonParameters.Gr'] <= lowerwslack));
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
            upperwslack = upperwslack - commonParameters.Gr(ind);
            lowerwslack = lowerwslack + commonParameters.Gr(ind);
        else
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
