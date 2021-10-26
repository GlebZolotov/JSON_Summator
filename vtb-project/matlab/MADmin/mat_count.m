function [count] = mat_count(fname)
    try
        load(fname, 'count');
        count = count + 1;
        fprintf('=================================================\n');
        fprintf('%s/count = %d\n', fname, count);
        fprintf('=================================================\n');
        save(fname, 'count', '-append','-nocompression');
    catch
        count = uint32(1);
        fprintf('=================================================\n');
        fprintf('%s/count = %d (NEW FILE)\n', fname, count);
        fprintf('=================================================\n');
        save(fname, 'count','-nocompression');
    end
end
