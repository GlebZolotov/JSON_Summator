classdef CommonParametersMADmin < CommonParameters
% this class contains the common parameters for the LP relaxations
% it is designed as handle class in order for the individual nodes to
% access it without having a copy in each node

    properties
        
        M double % for MAD: M = R - ones(T,1)*rbar, matrix in cost function for MADmin
        %minus_rho_div_Gr double % -rbar/Gr
        ind_rho_div_Gr uint16 % for auxiliary fast LP: sorted index list of ratio -rbar/Gr
        zeroTableau double % unnormalized simplex tableau coding the LP in the nodes
        
    end
    
    methods
        
        % CONSTRUCTOR        
        function obj = CommonParametersMADmin(bTcoef,mu0,wmin,R,Gr,rbar,numax,numshares,T)
            obj@CommonParameters(bTcoef,mu0,wmin,R,Gr,rbar,numax,numshares,T);
            obj.M = R - ones(T,1)*rbar;
            %obj.minus_rho_div_Gr = -rbar'./Gr;
            [~,ind] = sort(-rbar./Gr');
            obj.ind_rho_div_Gr = ind;
            % the tableau assumes that the slack of the (violated)
            % inequality sum(b) <= gammaMAD*T/2 is multiplied by -1
            % in order to minimize it in a first phase
            % the tableau is headed by a row coding the corresponding
            % auxiliary cost function
            obj.zeroTableau = [zeros(1,3*(numshares+1)+T), ones(1,T), 0; ...
                eye(numshares), -eye(numshares), zeros(numshares,numshares+3+2*T), -ones(numshares,1); ...
                eye(numshares), zeros(numshares), eye(numshares), zeros(numshares,3+2*T), ones(numshares,1); ...
                Gr', zeros(1,2*numshares), -1, zeros(1,2*(T+1)), wmin; ...
                Gr', zeros(1,2*numshares+1), 1, zeros(1,1+2*T), 1; ...
                rbar, zeros(1,2*(numshares+1)), -1, zeros(1,2*T), mu0; ...
                obj.M, zeros(T,2*numshares+3), -eye(T), eye(T), zeros(T,1)];

            dump_mat = 0;
            if (dump_mat)
                fname='dump/parameters.mat';
                count = mat_count(fname);
                obj.mat_save(fname, count);
            end
        end

        function mat_save_p(node, fname, count, prefix)
            mat_save_p(fname, 'params_bTcoef', node.bTcoef, count, prefix);
            mat_save_p(fname, 'params_mu0', node.mu0, count, prefix);
            mat_save_p(fname, 'params_wmin', node.wmin, count, prefix);
            mat_save_p(fname, 'params_R', node.R, count, prefix);
            mat_save_p(fname, 'params_Gr', node.Gr, count, prefix);
            mat_save_p(fname, 'params_numax', node.numax, count, prefix);
            mat_save_p(fname, 'params_W', node.numshares, count, prefix);
            mat_save_p(fname, 'params_T', node.T, count, prefix);

            mat_save_p(fname, 'params_rbar', node.rbar, count, prefix);
            mat_save_p(fname, 'params_M', node.M, count, prefix);
            mat_save_p(fname, 'params_ind_rho_div_Gr', node.ind_rho_div_Gr, count, prefix);
            mat_save_p(fname, 'params_zeroTableau', node.zeroTableau, count, prefix);
        end

        function mat_save(node, fname, count)
            node.mat_save_p(fname, count, '');
        end
    end
end