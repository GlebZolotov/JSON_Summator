classdef CommonParameters < handle
% this class contains the common parameters for the LP relaxations
% it is designed as handle class in order for the individual nodes to
% access it without having a copy in each node

    properties
        
        numshares uint16 % n = number of shares
        T uint16 % time horizon, number of days
        bTcoef double % for Cvar: bTcoef = 1/((1-beta)*T), coefficient at sum z_t in cost function
        mu0 double % minimal expected return
        wmin double % minimal fraction of capital to invest
        R double % matrix of returns, multiplied by fraction of lot, matrix T x n
        Gr double % fractions of lots, column vector
        rbar double % mean return, multiplied by fraction of lot, row vector
        numax uint16 % maximal number of lots of each share from pmax bound, integer column vector
        
    end
    
    methods
        
        % CONSTRUCTOR
        function obj = CommonParameters(bTcoef,mu0,wmin,R,Gr,rbar,numax,numshares,T)
            obj.numshares = numshares;
            obj.T = T;
            obj.bTcoef = bTcoef;
            obj.mu0 = mu0;
            obj.wmin = wmin;
            obj.R = R;
            obj.Gr = Gr;
            obj.rbar = rbar;
            obj.numax = numax;
        end
        
    end
    
end