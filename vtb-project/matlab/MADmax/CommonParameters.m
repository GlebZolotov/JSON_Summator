classdef CommonParameters < handle
% this class contains the common parameters for the LP relaxations
% it is designed as handle class in order for the individual nodes to
% access it without having a copy in each node

    properties
        
        numshares uint16 % n = number of shares
        T uint16 % time horizon, number of days
        wmin double % minimal fraction of capital to invest
        R double % matrix of returns, multiplied by fraction of lot, matrix T x n
        Gr double % fractions of lots, column vector
        rbar double % mean return, multiplied by fraction of lot, row vector
        numax uint16 % maximal number of lots of each share from pmax bound, integer column vector
        tolInt = 10^(-9) % tolerance for recognizing a real value to be an integer
        
    end
    
    methods
        
        % CONSTRUCTOR
        function obj = CommonParameters(wmin,R,Gr,rbar,numax,numshares,T)
            obj.numshares = numshares;
            obj.T = T;
            obj.wmin = wmin;
            obj.R = R;
            obj.Gr = Gr;
            obj.rbar = rbar;
            obj.numax = numax;
        end
        
    end
    
end