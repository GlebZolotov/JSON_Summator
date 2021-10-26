function [s,value,nu_low,nu_upp,nu,index,min_max,index1] = maximize_return_on_cube(commonParameters,nu_lower,nu_upper)
% function [s,value,nu_low,nu_upp,nu,index,min_max,index1] = maximize_return_on_cube(commonParameters,nu_lower,nu_upper)
%
% quick routine which maximizes rbar*nu over the cube {nu: nu_lower <= nu <= nu_upper} under
% the constraints wmin <= nu'*Gr <= 1
% dual variable is zero if unconditional maximum satisfies the constraints
% dual variable is positive or negative in dependence on whether the lower
% or upper constraint is violated
% in this case the dual variable to the constraints on nu'*Gr equals -rbar_i/Gr_i for
% some critical index i
% s is a boolean indicating whether the problem is feasible
% value is the optimal value
% nu_low,nu_upp are the lower and upper nearest integer vectors to the
% optimal nu
% index is the number of the fractional element (0 if there is none)
% index1 is the number of index in the sorted list ind_rho_div_Gr
% check if problem is infeasible
% min_max is a variable in {-1,0,+1} indicating whether the minimal value
% of Gr'*nu or the maximal or none is hit

dump_mat = 0;
if (dump_mat)
    fname='dump/maximize_return_on_cube.mat';
    count = mat_count(fname);

    mat_save(fname, 'in_Gr', commonParameters.Gr, count);
    mat_save(fname, 'in_rbar', commonParameters.rbar, count);
    mat_save(fname, 'in_ind_rho_div_Gr', commonParameters.ind_rho_div_Gr, count);
    mat_save(fname, 'in_W', commonParameters.numshares, count);
    mat_save(fname, 'in_wmin', commonParameters.wmin, count);
    mat_save(fname, 'in_nu_lower', double(nu_lower), count);
    mat_save(fname, 'in_nu_upper', double(nu_upper), count);
end

constr_val_lower = double(nu_lower)'*commonParameters.Gr;
constr_val_upper = double(nu_upper)'*commonParameters.Gr;
if (constr_val_lower > 1) || (constr_val_upper < commonParameters.wmin)
    s = false;
    value = -Inf;
    nu_low = [];
    nu_upp = [];
    nu = [];
    index = [];
    index1 = [];
    min_max = [];

    if (dump_mat)
        fprintf('__maximize_return_on_cube__1\n');
        mat_save(fname, 'out_s', double(s), count);
        mat_save(fname, 'out_value', value, count);
        mat_save(fname, 'out_nu_low', double(nu_low), count);
        mat_save(fname, 'out_nu_upp', double(nu_upp), count);
        mat_save(fname, 'out_nu', nu, count);
        mat_save(fname, 'out_index', int16(0), count);
        mat_save(fname, 'out_min_max', 0, count);
        mat_save(fname, 'out_index1', int16(0), count);
    end

    return;
end
s = true;
pos_c = find(commonParameters.rbar > 0)';
nu_c = double(nu_lower);
nu_c(pos_c) = double(nu_upper(pos_c)); % nu maximizing the objective unconditionally
c_value = commonParameters.Gr'*nu_c;
if (c_value >= commonParameters.wmin) && (c_value <= 1) % lambda = 0
    value = commonParameters.rbar*nu_c;
    nu_low = uint16(nu_c);
    nu_upp = nu_low;
    nu = nu_c;
    index = 0;
    index1 = [];
    min_max = 0;

    if (dump_mat)
        fprintf('__maximize_return_on_cube__2\n');
        mat_save(fname, 'out_s', double(s), count);
        mat_save(fname, 'out_value', value, count);
        mat_save(fname, 'out_nu_low', double(nu_low), count);
        mat_save(fname, 'out_nu_upp', double(nu_upp), count);
        mat_save(fname, 'out_nu', nu, count);
        mat_save(fname, 'out_index', int16(index), count);
        mat_save(fname, 'out_min_max', min_max, count);
        mat_save(fname, 'out_index1', int16(0), count);
    end

    return;
end
if c_value > 1 % unconditional constraint value too high, lambda < 0
    % scan through critical values from below
    index1 = 0;
    nu = double(nu_lower);
    constr_val = constr_val_lower;
    while constr_val <= 1
        index1 = index1 + 1;
        index = commonParameters.ind_rho_div_Gr(index1);
        nu(index) = double(nu_upper(index));
        constr_val = constr_val + (double(nu_upper(index)) - double(nu_lower(index)))*commonParameters.Gr(index);
    end
    nu(index) = double(nu_upper(index)) - (constr_val - 1)/commonParameters.Gr(index);
    min_max = 1;
else % unconditional constraint value too low, lambda > 0
    % scan through critical values from above
    index1 = commonParameters.numshares + 1;
    nu = double(nu_upper);
    constr_val = constr_val_upper;
    while constr_val >= commonParameters.wmin
        index1 = index1 - 1;
        index = commonParameters.ind_rho_div_Gr(index1);
        nu(index) = double(nu_lower(index));
        constr_val = constr_val - (double(nu_upper(index)) - double(nu_lower(index)))*commonParameters.Gr(index);
    end
    nu(index) = double(nu_lower(index)) + (commonParameters.wmin - constr_val)/commonParameters.Gr(index);
    min_max = -1;
end
value = commonParameters.rbar*nu;
if min_max ~= 0
    nu_low = nu;
    nu_low(index) = floor(nu_low(index));
    nu_low = uint16(nu_low);
    nu_upp = nu_low;
    nu_upp(index) = nu_upp(index) + 1;
end

if (dump_mat)
    fprintf('__maximize_return_on_cube__3\n');
    mat_save(fname, 'out_s', double(s), count);
    mat_save(fname, 'out_value', value, count);
    mat_save(fname, 'out_nu_low', double(nu_low), count);
    mat_save(fname, 'out_nu_upp', double(nu_upp), count);
    mat_save(fname, 'out_nu', nu, count);
    mat_save(fname, 'out_index', int16(index), count);
    mat_save(fname, 'out_min_max', min_max, count);
    mat_save(fname, 'out_index1', int16(index1), count);
end
