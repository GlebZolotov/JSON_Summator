#!/usr/bin/env python3

# Copyright (c) 2021 - present, Anton Anikin <anton@anikin.xyz>
# All rights reserved.

import numpy as np
from vtb import *


def main():
    np.random.seed(0)

    full_data = load_from_csv(
        meta_path="../data/vtb/meta.csv",
        close_path="../data/vtb/close.csv",
        returns_path="../data/vtb/returns.csv",
        covariance_path="../data/vtb/covariance.csv",
        verbose=True,
    )

    for _ in range(1):
        data = full_data
        # data = full_data.rand_W(150, verbose=True)

        for k in [20, 30, 40]:
        #     problem = MeanVarianceRiskMinimizationProblem(data)
            problem = MeanVarianceReturnMaximizationProblem(data)
            problem.K = k
            problem.save_mps("mv_rmin_w_{}_k_{}.mps".format(data.W, problem.K))

            d = problem.solve_with_gurobi(time_limit=2)
            # mat_save(d, "mv_rmin_w_{}_k_{}".format(data.W, problem.K), verbose=True)


if __name__ == '__main__':
    main()
