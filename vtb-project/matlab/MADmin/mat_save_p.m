function mat_save(fname, pname, pvalue, count, prefix)
    if prefix == ""
        pname = sprintf('r_%d_%s', count, pname);
    else
        pname = sprintf('r_%d_%s_%s', count, prefix, pname);
    end
    eval([pname '=  pvalue;']);
    save(fname, pname, '-append','-nocompression');
end
