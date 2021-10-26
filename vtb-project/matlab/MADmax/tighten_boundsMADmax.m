function [nu_lower,nu_upper] = tighten_boundsMADmax(commonParameters,nu_lower,nu_upper)
% function [nu_lower,nu_upper] = tighten_boundsMADmax(commonParameters,nu_lower,nu_upper)
%
% presolve for MAD max problem
% tightens the box bounds for the MAD max problem
% uses the two linear inequality constraints with non-integer coefficients
% the coefficients are taken from the commonParameters object
% the bounds are integer vectors
% tightening is repeated until no further progress is achieved
nu_lower = double(nu_lower);
nu_upper = double(nu_upper);
nu_lower_old = nu_lower;
nu_upper_old = nu_upper;
while true
    % tightening upper and lower bound by using linear constraints
    % on Gr'*nu
    gnhigh_delta = floor((1 - nu_lower'*commonParameters.Gr)./commonParameters.Gr);
    gnlow_delta = floor((nu_upper'*commonParameters.Gr - commonParameters.wmin)./commonParameters.Gr);
    nu_upper = min(nu_upper,nu_lower+gnhigh_delta);
    nu_lower = max(nu_lower,nu_upper-gnlow_delta);
    if (sum(nu_upper_old - nu_upper + nu_lower - nu_lower_old) == 0) || (min(nu_upper - nu_lower) < 0)
        break;
    end
    nu_lower_old = nu_lower;
    nu_upper_old = nu_upper;
end
nu_lower = uint16(nu_lower);
nu_upper = uint16(nu_upper);
