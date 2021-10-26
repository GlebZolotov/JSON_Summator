classdef CommonParametersMADmax < CommonParameters
% this class contains the common parameters for the LP relaxations
% it is designed as handle class in order for the individual nodes to
% access it without having a copy in each node

    properties
        
        M double % for MAD: M = R - ones(T,1)*rbar, matrix in cost function for MADmin
        gammaMAD double % for MADmax: maximal MAD variance
        ind_rho_div_Gr uint16 % for auxiliary fast LP: sorted index list of ratio -rbar/Gr
        inv_ind_rho_div_Gr uint16 % inverse map of ind_rho_div_Gr
        zeroTableau double % unnormalized simplex tableau coding the LP in the nodes
        
    end
    
    methods
        
        % CONSTRUCTOR
        function obj = CommonParametersMADmax(wmin,R,Gr,rbar,numax,numshares,T,gammaMAD)
            obj@CommonParameters(wmin,R,Gr,rbar,numax,numshares,T);
            obj.M = R - ones(T,1)*rbar;
            obj.gammaMAD = gammaMAD;
            [~,ind] = sort(-rbar./Gr');
            obj.ind_rho_div_Gr = ind;
            [~,iind] = sort(ind);
            obj.inv_ind_rho_div_Gr = iind;
            % the tableau assumes that the slack of the (violated)
            % inequality sum(b) <= gammaMAD*T/2 is multiplied by -1
            % in order to minimize it in a first phase
            % the tableau is headed by a row coding the corresponding
            % auxiliary cost function
            obj.zeroTableau = [zeros(1,3*numshares+2), 1, zeros(1,2*T+1); ...
                -rbar, zeros(1,2*(numshares+T)+4); ...
                eye(numshares), -eye(numshares), zeros(numshares,numshares+4+2*T); ...
                eye(numshares), zeros(numshares), eye(numshares), zeros(numshares,4+2*T); ...
                Gr', zeros(1,2*numshares), -1, zeros(1,2*(T+1)), wmin; ...
                Gr', zeros(1,2*numshares+1), 1, zeros(1,1+2*T), 1; ...
                zeros(1,3*numshares+2), -1, zeros(1,T), ones(1,T), gammaMAD*double(T)/2; ...
                obj.M, zeros(T,2*numshares+3), -eye(T), eye(T), zeros(T,1)];
        end
        
    end
    
end