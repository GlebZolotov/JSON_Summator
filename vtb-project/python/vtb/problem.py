# Copyright (c) 2021 - present, Anton Anikin <anton@anikin.xyz>
# All rights reserved.

import numpy as np
from typing import Optional

from vtb.data import Data

__all__ = ["Parameters"]


class Parameters:
    # Minimum expected return
    #
    # + MAD Risk Minimization
    # - MAD Return Maximization
    # + Mean-Variance Risk Minimization
    # + Mean-Variance Return Maximization
    mu: float = 0.0016

    # Trust level
    #
    beta: float = 0.95

    # Risk limitation
    #
    # + MAD Risk Minimization
    # + MAD Return Maximization
    gamma_mad: float = 0.005

    # Risk limitation
    #
    gamma_cvar: float = 0.05

    # Risk limitation
    #
    # - Mean-Variance Risk Minimization
    # + Mean-Variance Return Maximization
    sigma: float = 0.0126

    # Client's capital
    #
    # + MAD Risk Minimization
    # + MAD Return Maximization
    # + Mean-Variance Risk Minimization
    # + Mean-Variance Return Maximization
    capital: float = 1e6

    # Upper limit on the weight of the stock
    #
    # + MAD Risk Minimization
    # + MAD Return Maximization
    # + Mean-Variance Risk Minimization
    # + Mean-Variance Return Maximization
    p_max: float = 0.15

    # Minimum allocation in stocks
    #
    # + MAD Risk Minimization
    # + MAD Return Maximization
    # + Mean-Variance Risk Minimization
    # + Mean-Variance Return Maximization
    w_min: float = 0.9

    # Maximum number of stocks in the portfolio
    #
    # + Mean-Variance Risk Minimization
    # + Mean-Variance Return Maximization
    K: int = 20


class Problem:
    def __init__(
            self,
            data: Data):

        self._W = data.W
        self._close = data.close
        self._returns = data.returns
        self._lot_size = data.lot_size

        self.capital = Parameters.capital

    def _gr(self) -> np.ndarray:
        return self._lot_size * self._close[0].flatten() / self.capital

    def _rbar_plain(self) -> np.ndarray:
        return self._returns.mean(0)

    def _rbar(self, gr: np.ndarray) -> np.ndarray:
        return self._returns.mean(0) * gr
