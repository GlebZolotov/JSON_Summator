import scipy.io
import pandas as pd
import sys

from_file_path = sys.argv[1]
to_file_path = sys.argv[2]

mat = scipy.io.loadmat(from_file_path)
mat = {k:v for k, v in mat.items() if k[0] != '_'}

data_map = dict()
for k, v in mat.items():
    if(len(v) == 1):
        data_map[k] = pd.Series(v[0])
    else:
        for ind, val in enumerate(v):
            key_val = k+"_"+str(ind)
            data_map[key_val]=pd.Series(val)

data = pd.DataFrame(data_map)
data.to_csv(to_file_path, index_label="Index")
