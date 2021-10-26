#!/usr/bin/env python3

# Copyright (c) 2021 - present
#
# Anton Anikin <anton@anikin.xyz>
# Alexander Gornov <gornov.a.yu@gmail.com>
#
# All rights reserved.

import os
import sys
import h5py
from scipy.io import loadmat


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} file.mat [file2.mat] ...")
        exit(0)

    for mat_name in sys.argv[1:]:
        hdf_name = os.path.splitext(mat_name)[0] + ".h5"
        print(mat_name, "-->", hdf_name)

        try:
            hdf_file = h5py.File(hdf_name, "w")

            for var_name, var_value in loadmat(mat_name, mat_dtype=True).items():
                if var_name.startswith("_"):
                    continue

                # print(var_name, var_value.shape, var_value.dtype)
                hdf_file.create_dataset(var_name, data=var_value)

            hdf_file.close()

        except Exception as e:
            print("    ", e)
            os.remove(hdf_name)
