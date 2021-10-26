# Copyright (c) 2021 - present, Anton Anikin <anton@anikin.xyz>
# All rights reserved.

import numpy as np


# FIXME remove
def is_int(array: np.ndarray) -> bool:
    return (array == array.astype(np.int)).all()


def round_check(array: np.ndarray) -> bool:
    r = array.round()
    assert np.linalg.norm(array - r) <= 1e-9
    return r
