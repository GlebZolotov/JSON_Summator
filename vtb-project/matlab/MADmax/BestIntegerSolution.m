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
        
    end
    
end