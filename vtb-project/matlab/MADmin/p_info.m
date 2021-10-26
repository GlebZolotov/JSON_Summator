function p_info(value, varargin)
if true
    if size(varargin) == 0
        fprintf("%15s : %s %s\n", inputname(1), class(value), mat2str(size(value)))
    else
        fprintf("%15s : %s %s\n", varargin{1}, class(value), mat2str(size(value)))
    end
end
