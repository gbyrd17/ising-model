import os
import struct

import numpy as np

expected_out_path = "./output/sim_output.bin"


def read_bin():
    if not os.path.exists(expected_out_path):
        print(f"!!Error!! {expected_out_path} not found.")
        return None

    with open(expected_out_path, "rb") as f:
        # two doubles, two ints ==> (2*8) + (2*4) = 24 bytes
        header = f.read(24)
        energy, mag, rows, cols = struct.unpack("ddii", header)

        total_sites = rows * cols
        ## spins stored as int8_t ==> 1 byte each
        spins_raw = f.read(total_sites)
        spins = np.frombuffer(spins_raw, dtype=np.int8)
        grid = spins.reshape(rows, cols)

        if spins.size != total_sites:
            print(f"Warning: Expected {total_sites} spin sites, but found {spins.size}")

        print(grid)
        print("binary read successfully!")
        return {
            "energy": energy,
            "magnetization": mag,
            "rows": rows,
            "cols": cols,
            "grid": grid,
        }
