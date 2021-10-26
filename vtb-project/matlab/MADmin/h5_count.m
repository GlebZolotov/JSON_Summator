function [count] = h5_count(fname)
    try
        count = h5read(fname, '/count') + 1;
        fprintf('%s/count = %d\n', fname, count);
    catch
        count = 1;
        fprintf('%s/count = FAIL\n', fname);
        h5create(fname, '/count', [1]);
    end
    h5write(fname, '/count', count);
end
