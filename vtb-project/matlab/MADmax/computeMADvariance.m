function MADvar = computeMADvariance(commonParameters,nu)
% function MADvar = computeMADvariance(commonParameters,nu)
%
% computes MAD variance for given nu
MADvar = sum(abs(commonParameters.M*double(nu)))/double(commonParameters.T);