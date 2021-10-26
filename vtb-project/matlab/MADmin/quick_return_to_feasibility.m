function [s,Tab,Bas] = quick_return_to_feasibility(delta,j,q,Tab,Bas)
% function [s,Tab,Bas] = quick_return_to_feasibility(delta,j,q,Tab,Bas)
%
% in the linear system corresponding to the simplex table Tab the basic
% variable j, corresponding to row q, has been changed by delta
% this can be corrected by adjusting the right-hand side of equation q
% if the table becomes infeasible, then a phase 1 like optimization is
% performed to return slack j to zero
% s is a boolean indicating whether a feasible point has been reached

dump_mat = 0;
if (dump_mat)
    fname='dump/quick_return_to_feasibility.mat';
    count = mat_count(fname);

    assert(norm(j' - double(uint16(j'))) == 0.0);
    assert(norm(q' - double(uint16(q'))) == 0.0);

    mat_save(fname, 'in_delta', delta, count);
    mat_save(fname, 'in_j', j, count);
    mat_save(fname, 'in_q', q, count);
    mat_save(fname, 'in_Tab', Tab, count);
    mat_save(fname, 'in_Bas', Bas, count);
end

Tab(q+1,end) = Tab(q+1,end) + delta;
if Tab(q+1,end) < 0
    Tab(q+1,:) = -Tab(q+1,:);
    Tab(:,j) = -Tab(:,j);
    % adding row corresponding to auxiliary cost function
    Tab = [-Tab(q+1,:); Tab];
    Tab(1,j) = 0;
    % solving auxiliary program
    opt = false;
    count_advance = 0;
    start = tic();
    while ~opt
        count_advance = count_advance + 1;
        [opt,~,Tab,Bas] = advance_tableau(Tab,Bas,true);
    end
    opt_time = toc(start) * 1000.0;
    fprintf("QR Phase 1 from parent table: %d simplex iterations\n",count_advance);
    fprintf("in %.2f ms.; %.4f ms. per iteration\n", opt_time, opt_time/count_advance);
    if Tab(1,end) < -10^(-12)
        s = false;
        Tab = [];
        Bas = [];
    else
        s = true;
        Tab(1,:) = [];
        Tab(:,j) = -Tab(:,j);
        q = find(Bas == j,1);
        if ~isempty(q) % variable j is still basic
            Tab(q+1,:) = -Tab(q+1,:);
            Tab(q+1,end) = 0;
        end
    end
else
    s = true;
end

if (dump_mat)
    mat_save(fname, 'out_s', double(s), count);
    mat_save(fname, 'out_Tab', Tab, count);
    mat_save(fname, 'out_Bas', Bas, count);
end
