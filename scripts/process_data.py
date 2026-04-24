import os

import numpy as np

expected_out_path = "./output/sim_output.bin"


def read_bin():
    if not os.path.exists(expected_out_path):
        print(f"!!Error!! {expected_out_path} not found.")
        return None

    with open(expected_out_path, "rb") as f:
        ## energy and mag stored as double ==> 8 bytes each
        energy = np.frombuffer(f.read(8), dtype=np.float64)[0]
        mag = np.frombuffer(f.read(8), dtype=np.float64)[0]
        ## rows and cols stored as int ==> 4 bytes each
        rows = np.frombuffer(f.read(4), dtype=np.int32)[0]
        cols = np.frombuffer(f.read(4), dtype=np.int32)[0]

        total_sites = rows * cols
        ## spins stored as int8_t ==> 1 byte each
        spins = np.frombuffer(f.read(total_sites), dtype=np.int8)
        grid = spins.reshape((rows, cols))
        return {
            "energy": energy,
            "magnetization": mag,
            "rows": rows,
            "cols": cols,
            "grid": grid,
        }
