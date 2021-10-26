# Copyright (c) 2021 - present, Anton Anikin <anton@anikin.xyz>
# All rights reserved.

import numpy as np
import pandas
import struct
from typing import Dict

__all__ = ["Data", "load_from_csv", "bin_save", "bin_save_1", "mat_save"]


class Data:
    def __init__(
            self,
            shares: np.ndarray,
            lot_size: np.ndarray,
            close: np.ndarray,
            returns: np.ndarray,
            covariance: np.ndarray,
            verbose: bool = False
    ):
        self.shares = shares.flatten()
        self.W = shares.shape[0]

        self.lot_size = lot_size.flatten()
        assert self.lot_size.shape[0] == self.W

        self.close = close
        assert self.close.shape[1] == self.W

        self.returns = returns
        self.T = self.returns.shape[0]
        assert self.returns.shape[1] == self.W

        self.covariance = covariance
        assert self.covariance.shape[0] == self.W
        assert self.covariance.shape[1] == self.W
        assert np.linalg.norm(self.covariance - self.covariance.T) < 1e-9

        if verbose:
            def print1(a: np.ndarray, name: str):
                if len(a.shape) == 1:
                    print("  {:10} : {:4} x {}, {}".format(name, a.shape[0], 1, a.dtype))
                else:
                    print("  {:10} : {:4} x {}, {}".format(name, a.shape[0], a.shape[1], a.dtype))

            print()
            print("Data W =", self.W)
            print("Data T =", self.T)
            print1(self.shares, "shares")
            print1(self.lot_size, "lot_size")
            print1(self.close, "close")
            print1(self.returns, "returns")
            # print1(self.rbar, "rbar")
            # print1(self.gr, "gr")
            print1(self.covariance, "covariance")

    def rand_W(self, new_W: int, verbose: bool = False) -> "Data":
        assert new_W > 1
        assert new_W < self.W

        idx = np.arange(self.W)
        np.random.shuffle(idx)
        idx = idx[:new_W]

        return Data(
            shares=self.shares[idx],
            lot_size=self.lot_size[idx],
            close=self.close[:, idx],
            returns=self.returns[:, idx],
            covariance=self.covariance[idx, :][:, idx],
            verbose=verbose)


def load_from_csv(
        meta_path: str,
        close_path: str,
        returns_path: str,
        covariance_path: str,
        verbose: bool = False) -> Data:

    def is_symmetric(df: pandas.DataFrame) -> bool:
        return \
            (df.index == df.columns).all() and \
            (df.to_numpy() == df.to_numpy().T).all()

    meta: pandas.DataFrame = pandas.read_csv(meta_path, header=0, index_col=0)
    meta = meta.transpose()
    meta.sort_index(axis=1, ascending=True, inplace=True)

    close: pandas.DataFrame = pandas.read_csv(close_path, header=0, index_col=0)
    close.loc[close.index[0], :].fillna(value=0.0, inplace=True)
    close.fillna(method="ffill", inplace=True)
    close.sort_index(axis=0, ascending=False, inplace=True)
    close.sort_index(axis=1, ascending=True, inplace=True)

    returns: pandas.DataFrame = pandas.read_csv(returns_path, sep=",", header=0, index_col=0)
    returns.sort_index(axis=0, ascending=False, inplace=True)
    returns.sort_index(axis=1, ascending=True, inplace=True)

    covariance: pandas.DataFrame = pandas.read_csv(covariance_path, sep=",", header=0, index_col=0)
    assert is_symmetric(covariance)

    covariance.sort_index(axis=0, ascending=True, inplace=True)
    covariance.sort_index(axis=1, ascending=True, inplace=True)
    assert is_symmetric(covariance)

    shares = \
        set(meta.columns) & \
        set(close.columns) & \
        set(returns.columns) & \
        set(covariance.columns)

    def drop_unused_shares(df: pandas.DataFrame, name: str, drop_index: bool = False):
        unused_shares = list(set(df.columns) - shares)
        df.drop(columns=unused_shares, inplace=True)

        if verbose:
            print("  {:11}: -{}".format(name, len(unused_shares)))

        if drop_index:
            df.drop(index=unused_shares, inplace=True)

    if verbose:
        print()
        print("CSV shares count = {}, drop unused:".format(len(shares)))
    drop_unused_shares(meta, "meta")

    drop_unused_shares(close, "close")
    assert (meta.columns == close.columns).all()

    drop_unused_shares(returns, "returns")
    assert (meta.columns == returns.columns).all()

    drop_unused_shares(covariance, "covariance", drop_index=True)
    assert (meta.columns == covariance.columns).all()
    assert is_symmetric(covariance)

    return Data(
        shares=meta.columns.to_numpy(dtype=np.str_),
        lot_size=meta.loc["LOTSIZE", :].to_numpy(dtype=np.float),
        close=close.to_numpy(),
        returns=returns.to_numpy(),
        covariance=covariance.to_numpy(),
        verbose=verbose
    )


def _print_data_dict(data: Dict):
    keys = list(data.keys())
    keys.sort()
    width = max(map(len, keys))
    for key in keys:
        print("  {:{width}} : ".format(key, width=width), end="")

        value = data[key]
        if type(value) == np.ndarray:
            shape = list(value.shape)
            if len(shape) == 1:
                shape.append(1)
            shape = " x ".join(map(str, shape))
            print("{}, {}".format(value.dtype, shape))
        else:
            print(type(value).__name__)


def _fix_out_dir(out_dir: str) -> str:
    if out_dir == "":
        return "./"

    # if not out_dir.startswith("./"):
    #     out_dir = "./" + out_dir

    if not out_dir.endswith("/"):
        out_dir = out_dir + "/"

    return out_dir


def bin_save_1(array: np.ndarray, array_name: str, out_dir: str = ".", verbose: bool = False):
    bin_save({array_name: array}, out_dir, verbose)


def bin_save(data: Dict, out_dir: str = "./", verbose: bool = False):
    out_dir = _fix_out_dir(out_dir)
    for old_key in list(data.keys()):
        new_key = "{}{}.bin".format(out_dir, old_key)
        data[new_key] = data.pop(old_key)

    if verbose:
        print()
        print("save binary (C/C++) data :".format(out_dir))
        _print_data_dict(data)

    for file_name in data.keys():
        array = data[file_name]
        assert type(array) == np.ndarray  # FIXME

        f = open(file_name, "wb")
        for s in array.shape:
            f.write(struct.pack("i", s))

        array.tofile(f)
        f.close()


def mat_save(data: Dict, data_name: str, out_dir: str = "./", verbose: bool = False):
    import scipy.io as sio

    out_dir = _fix_out_dir(out_dir)
    file_name = "{}{}.mat".format(out_dir, data_name)
    if verbose:
        print()
        print("save data into {} :".format(file_name))
        _print_data_dict(data)

    sio.savemat(file_name, data)
