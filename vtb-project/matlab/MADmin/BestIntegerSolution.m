classdef BestIntegerSolution < handle
% this class contains the best integer solution found so far
% it is designed as handle class in order for the individual nodes to be
% able to change it globally for all nodes if a better solution is found

    properties
        
        upperBound double
        bestNu uint16
        
    end
    
    methods
        
        function setValues(obj,opt_value,nu)
            obj.upperBound = opt_value;
            if ~isinf(opt_value)
                obj.bestNu = nu;
            end
        end

        function mat_save(sol, fname, count)
            sol.mat_save_p(fname, count, '')
        end

        function mat_save_p(sol, fname, count, prefix)
            mat_save_p(fname, 'bis_upperBound', sol.upperBound, count, prefix);
            mat_save_p(fname, 'bis_bestNu', double(sol.bestNu), count, prefix);
        end
    end
    
end