function [nu_lower,nu_upper] = tighten_boundsMADmin(commonParameters,nu_lower,nu_upper)
% function [nu_lower,nu_upper] = tighten_boundsMADmin(commonParameters,nu_lower,nu_upper)
%
% presolve for MAD min problem
% tightens the box bounds for the MAD min problem
% uses the three linear inequality constraints with non-integer coefficients
% the coefficients are taken from the commonParameters object
% the bounds are integer vectors
% tightening is repeated until no further progress is achieved
% tightening the bounds using this (single inequality) way
% can later be replaced by LPs minimizing and maximizing
% individual integer variables

dump_mat = 0;
if (dump_mat)
    fname='dump/tighten_boundsMADmin.mat';
    count = mat_count(fname);

    mat_save(fname, 'in_W', commonParameters.numshares, count);
    mat_save(fname, 'in_rbar', commonParameters.rbar, count);
    mat_save(fname, 'in_Gr', commonParameters.Gr, count);
    mat_save(fname, 'in_wmin', commonParameters.wmin, count);
    mat_save(fname, 'in_mu0', commonParameters.mu0, count);
    mat_save(fname, 'in_nu_lower_int', nu_lower, count);
    mat_save(fname, 'in_nu_upper_int', nu_upper, count);
end

nu_lower = double(nu_lower);
nu_upper = double(nu_upper);
if (dump_mat)
    mat_save(fname, 'in_nu_lower_double', nu_lower, count);
    mat_save(fname, 'in_nu_upper_double', nu_upper, count);
end

nu_lower_old = nu_lower;
nu_upper_old = nu_upper;
indm = find(commonParameters.rbar < 0);
indp = find(commonParameters.rbar > 0);
while true
    % tightening upper and lower bound by using linear constraints
    % on Gr'*nu
    gnhigh_delta = floor((1 - nu_lower'*commonParameters.Gr)./commonParameters.Gr);
    gnlow_delta = floor((nu_upper'*commonParameters.Gr - commonParameters.wmin)./commonParameters.Gr);
    nu_upper = min(nu_upper,nu_lower+gnhigh_delta);
    nu_lower = max(nu_lower,nu_upper-gnlow_delta);
    % tightening upper and lower bound by using linear constraints
    % on rbar*nu
    mu0_delta = commonParameters.rbar(indm)*nu_lower(indm) + commonParameters.rbar(indp)*nu_upper(indp) - commonParameters.mu0;
    nu_lower(indp) = max(nu_lower(indp),nu_upper(indp)-floor(mu0_delta./(commonParameters.rbar(indp))'));
    nu_upper(indm) = min(nu_upper(indm),nu_lower(indm)+floor(mu0_delta./(-commonParameters.rbar(indm))'));
    if (sum(nu_upper_old - nu_upper + nu_lower - nu_lower_old) == 0) || (min(nu_upper - nu_lower) < 0)
        break;
    end
    nu_lower_old = nu_lower;
    nu_upper_old = nu_upper;
end

if (dump_mat)
    mat_save(fname, 'out_nu_lower_double', nu_lower, count);
    mat_save(fname, 'out_nu_upper_double', nu_upper, count);
end

nu_lower = uint16(nu_lower);
nu_upper = uint16(nu_upper);

if (dump_mat)
    mat_save(fname, 'out_nu_lower_int', nu_lower, count);
    mat_save(fname, 'out_nu_upper_int', nu_upper, count);
end
