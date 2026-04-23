import json
import os

import numpy as np


def generate_config(rows, cols, temp=2.26, j=1.0, out_dir="./data/sim_01"):
    # init vars for use in configuration
    os.makedirs(out_dir, exist_ok=True)

    fields = np.zeros((rows, cols), dtype=np.float64)

    h_couplings = np.full((rows, cols), j, dtype=np.float64)
    v_couplings = np.full((rows, cols), j, dtype=np.float64)

    # -- user input section --
    # Example = let field be stron on left half
    fields[:, : cols // 2] = 0.5

    # Example = incorperate a defect along the lattice where coupling weakens
    h_couplings[rows // 2, :] = 0.1

    # -- making binary maps then || to config.json --
    field_path = os.path.join(out_dir, "fields.bin")
    h_j_path = os.path.join(out_dir, "h_coupling.bin")
    v_j_path = os.path.join(out_dir, "v_coupling.bin")

    fields.tofile(field_path)
    h_couplings.tofile(h_j_path)
    v_couplings.tofile(v_j_path)

    config = {
        "rows": rows,
        "cols": cols,
        "T": temp,
        "field_map": field_path,
        "h_coupling_map": h_j_path,
        "v_coupling_map": v_j_path,
        "output_dir": out_dir,
    }
    with open("config.json", "w") as f:
        json.dump(config, f, indent=2)

        print(f"Config and binary maps successfully generated in {out_dir}.")


if __name__ == "__main__":
    generate_config(rows=64, cols=64)
