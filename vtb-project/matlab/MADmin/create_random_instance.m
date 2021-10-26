function commonParameters = create_random_instance
% function commonParameters = create_random_instance
%
% creates random problem instance
% returns directly a CommonParameter object
load sharedata_reduced Gran RetG
% choosing random selection of shares from the sample of 200
% number of shares in [100,200]
numshares = 100 + round(100*rand);
% indices of selected shares
ind_shares = (1:numshares) + round((200-numshares)*sort(rand(1,numshares)));
% maximal weight of each share, in [0.1,0.3]
pmax = 0.1 + rand*0.2;
% T length of history, in [100,300]
T = 100 + round(rand*200);
% mu0 lower bound on returns, in [0.0001,0.002]
mu0 = 0.0001 + rand*0.0019;
% wmin = min weight of shares in capital, in [0.9,0.99]
wmin = 0.9 + rand*0.09;
% beta confidence level
beta = 0.95;
% preparing the fixed matrices and vectors
% R are the returns, multiplied by the granularities
R = RetG(end-T+1:end,ind_shares);
% Gr are the granularities, column vector
Gr = Gran(ind_shares)'; % column vector
% rbar are the means over t of R, a row vector
rbar = mean(R);
% maximal number of lots for each share, integer column vector
numax = uint16(floor(pmax*ones(numshares,1)./Gr));
% bTcoef = 1/((1-beta)*T)
bTcoef = 1/((1-beta)*T);
% create CommonParameter object
clear Gran RetG
commonParameters = CommonParametersMADmin(bTcoef,mu0,wmin,R,Gr,rbar,numax,numshares,T);
