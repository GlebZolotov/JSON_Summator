# Copyright (c) 2021 - present, Anton Anikin <anton@anikin.xyz>
# All rights reserved.

import abc
import numpy as np
from typing import Dict, Tuple

from vtb.data import Data, bin_save
from vtb.problem import Problem, Parameters
from vtb.utils import round_check

__all__ = ["MeanVarianceRiskMinimizationProblem", "MeanVarianceReturnMaximizationProblem"]


class MeanVarianceProblem(Problem, metaclass=abc.ABCMeta):
    def __init__(self, data: Data):
        super().__init__(data)

        self.mu = Parameters.mu
        self.w_min = Parameters.w_min
        self.p_max = Parameters.p_max
        self.K = Parameters.K

        self._covariance = data.covariance

        # variables
        self.y = np.zeros(self._W, dtype=np.float64)
        self.nl = np.zeros(self._W, dtype=np.float64)

    def _gr_rbar_C(self) -> Tuple[np.ndarray, np.ndarray, np.ndarray]:
        gr = self._gr()
        rbar = self._rbar(gr)

        # column-wise and row-wise multiplication
        C = self._covariance * gr * gr[:, None]
        assert np.linalg.norm(C - C.T) < 1e-9

        return gr, rbar, C

    @abc.abstractmethod
    def _gurobi_model(self) -> "Tuple[gurobipy.Model,gurobipy.MVar,gurobipy.MVar]":
        pass

    def save_bin(self, out_dir: str = ".", verbose: bool = False):
        gr, rbar, C = self._gr_rbar_C()
        bin_save({"gr": gr, "rbar": rbar, "covariance": C}, out_dir, verbose)

    def save_mps(self, file_name: str):
        model, _, _ = self._gurobi_model()
        model.write(file_name)

    def solve_with_gurobi(
            self,
            time_limit: float = 1e6,
            threads: int = 1,
            verbose: bool = False) -> Dict:
        import gurobipy as gp
        from gurobipy import GRB

        output = {"K": self.K, "W": self._W}

        model, nl, y = self._gurobi_model()

        # model.setParam(GRB.Param.TimeLimit, 2.0)
        # model.setParam(GRB.Param.Threads, 1)
        model.setParam(gp.ParamConstClass.TimeLimit, time_limit)
        model.setParam(gp.ParamConstClass.Threads, threads)

        model.optimize()

        status = model.getAttr(gp.AttrConstClass.Status)
        output["status"] = status
        output["runtime"] = model.getAttr(gp.AttrConstClass.Runtime)

        # https://www.gurobi.com/documentation/9.1/refman/optimization_status_codes.html#sec:StatusCodes
        if status == GRB.INF_OR_UNBD:
            return output

        output["obj"] = model.getAttr(gp.AttrConstClass.ObjVal)
        output["obj_bound"] = model.getAttr(gp.AttrConstClass.ObjBound)
        output["mipgap"] = model.getAttr(gp.AttrConstClass.MIPGap)

        self.y = round_check(y.X)
        output["y"] = self.y

        self.nl = round_check(nl.X)
        output["nl"] = self.nl

        y_nrm0 = np.linalg.norm(self.y, 0)
        nl_nrm1 = np.linalg.norm(self.nl, 1)
        output["y_nrm0"] = y_nrm0
        output["nl_nrm1"] = nl_nrm1

        # for v in model.getVars():
        #     if v.x > 0:
        #         print('{} = {};'.format(v.varName, v.x))

        for i in range(self._W):
            if verbose:
                if self.y[i] != 0.0:
                    print('y[{}] = {};'.format(i, self.y[i]))

        for i in range(self._W):
            if verbose:
                if self.nl[i] != 0.0:
                    print('nl[{}] = {};'.format(i, self.nl[i]))

        print()
        print("obj       = {:.16e}".format(output["obj"]))
        print("obj_bound = {:.16e}".format(output["obj_bound"]))
        print("y  L0 norm = {}".format(y_nrm0))
        print("nl L1 norm = {}".format(nl_nrm1))

        return output


class MeanVarianceRiskMinimizationProblem(MeanVarianceProblem):
    def __init__(self, data: Data):
        super().__init__(data)

    def _gurobi_model(self) -> "Tuple[gurobipy.Model,gurobipy.MVar,gurobipy.MVar]":
        import gurobipy as gp
        from gurobipy import GRB

        gr, rbar, C = self._gr_rbar_C()

        model = gp.Model("Mean-Variance Risk Minimization")

        nl: gp.MVar = model.addMVar(shape=(self._W,), vtype=GRB.INTEGER, name="nl")
        y: gp.MVar = model.addMVar(shape=(self._W,), vtype=GRB.BINARY, name="y")

        obj = nl @ C @ nl
        model.setObjective(obj, GRB.MINIMIZE)

        model.addConstr(nl @ gr <= 1.0, "w_sum_max")
        model.addConstr(nl @ gr >= self.w_min, "w_sum_min")
        model.addConstr(self.mu <= nl @ rbar, "w_rbar_mu")
        model.addConstr(y.sum() <= self.K, "y_sum")
        model.addConstr(np.diag(gr) @ nl <= y * self.p_max, "w_y_p_max")

        return model, nl, y


class MeanVarianceReturnMaximizationProblem(MeanVarianceProblem):
    def __init__(self, data: Data):
        super().__init__(data)

        self.sigma = Parameters.sigma

    def _gurobi_model(self) -> "Tuple[gurobipy.Model,gurobipy.MVar,gurobipy.MVar]":
        import gurobipy as gp
        from gurobipy import GRB

        gr, rbar, C = self._gr_rbar_C()

        model = gp.Model("Mean-Variance Return Maximization")

        nl: gp.MVar = model.addMVar(shape=(self._W,), vtype=GRB.INTEGER, name="nl")
        y: gp.MVar = model.addMVar(shape=(self._W,), vtype=GRB.BINARY, name="y")

        obj = nl @ rbar
        model.setObjective(obj, GRB.MAXIMIZE)

        model.addConstr(nl @ gr <= 1.0, "w_sum_max")
        model.addConstr(nl @ gr >= self.w_min, "w_sum_min")
        model.addConstr(nl @ C @ nl <= self.sigma, "w_C_w_sigma")
        model.addConstr(y.sum() <= self.K, "y_sum")
        model.addConstr(np.diag(gr) @ nl <= y * self.p_max, "w_y_p_max")

        return model, nl, y
