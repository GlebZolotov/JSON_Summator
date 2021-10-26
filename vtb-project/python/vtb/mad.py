# Copyright (c) 2021 - present, Anton Anikin <anton@anikin.xyz>
# All rights reserved.

import numpy as np
from typing import Dict, Tuple

from vtb.data import Data, bin_save
from vtb.problem import Problem, Parameters
from vtb.utils import is_int

__all__ = ["MADRiskMinimizationProblem"]


class MADProblem(Problem):
    def __init__(self,data: Data):
        super().__init__(data)

        self.w_min = Parameters.w_min
        self.p_max = Parameters.p_max

        self._T = data.T

        # variables
        self.nl = np.zeros(self._W, dtype=np.float64)
        self.a = np.zeros(self._T, dtype=np.float64)
        self.b = np.zeros(self._T, dtype=np.float64)

    def _rr(self, gr:np.ndarray, rbar: np.ndarray) -> np.ndarray:
        return self._returns * gr - rbar

    def _gr_rbar_rr(self) -> Tuple[np.ndarray, np.ndarray, np.ndarray]:
        gr = self._gr()
        rbar = self._rbar_plain()
        rr = self._returns - rbar

        return gr, rbar * gr, rr * gr

    def save_to_bin(self, out_dir: str = ".", verbose: bool = False):
        gr, rbar, rr = self._gr_rbar_rr()
        bin_save({"gr": gr, "rbar": rbar, "rr": rr}, out_dir, verbose)


class MADRiskMinimizationProblem(MADProblem):
    def __init__(self, data: Data):
        super().__init__(data)

        self.mu = Parameters.mu

    def _gurobi_model(self) -> "Tuple[gurobipy.Model,gurobipy.MVar,gurobipy.MVar,gurobipy.MVar]":
        import gurobipy as gp
        from gurobipy import GRB

        gr, rbar, rr = self._gr_rbar_rr()

        model = gp.Model("MAD Risk Minimization")

        nl: gp.MVar = model.addMVar(shape=(self._W,), vtype=GRB.INTEGER, name="nl")
        a: gp.MVar = model.addMVar(shape=(self._T,), name="a")
        b: gp.MVar = model.addMVar(shape=(self._T,), name="b")

        obj = b.sum()
        model.setObjective(obj, GRB.MINIMIZE)

        model.addConstr(gr @ nl <= 1.0, "w_sum_max")
        model.addConstr(gr @ nl >= self.w_min, "w_sum_min")
        model.addConstr(self.mu <= rbar @ nl, "w_rbar_mu")
        model.addConstr(b + rr @ nl == a, "b_r_rbar_w_a")
        model.addConstr(np.diag(gr) @ nl <= self.p_max, "w_p_max")

        return model, nl, a, b

    def to_mps(self, file_name: str):
        model, _, _, _ = self._gurobi_model()
        model.write(file_name)

    def solve_with_gurobi(
            self,
            time_limit: float = 1e6,
            threads: int = 1) -> Dict:

        import gurobipy as gp
        from gurobipy import GRB

        output = {}

        model, nl, a, b = self._gurobi_model()

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

        self.nl = nl.X
        assert is_int(self.nl)
        output["nl"] = self.nl

        self.a = a.X
        output["a"] = self.a

        self.b = b.X
        output["b"] = self.b

        print()
        print("obj       = {:.16e}".format(output["obj"]))
        print("obj_bound = {:.16e}".format(output["obj_bound"]))

        return output


# class MeanVarianceReturnMaximizationProblem(MeanVarianceProblem):
#     def __init__(self, data: Data):
#         super().__init__(data)
#
#         self.sigma = Parameters.sigma
#
#     def solve_with_gurobi(
#             self,
#             time_limit: float = 1e6,
#             threads: int = 1) -> Dict:
#
#         # FIXME
#         pass
