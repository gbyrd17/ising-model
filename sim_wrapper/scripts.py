import os
import struct
import subprocess

import matplotlib.pyplot as plt
import numpy as np
from matplotlib.animation import FuncAnimation


class Simulation:
    def __init__(self, frames):
        self.path = "./output/sim_output.bin"
        self.frames = frames

    def get_data(self):
        if not os.path.exists(self.path):
            print(f"!!Error!! {self.path} not found.")
            return False

        file_size = os.path.getsize(self.path)

        with open(self.path, "rb") as f:
            header = f.read(24)
            if not header:
                return False
            _, _, rows, cols = struct.unpack("ddii", header)
            f.seek(0)  # reset to start

            frame_size = 24 + (rows * cols)
            expected_total = frame_size * self.frames

            if file_size < expected_total:
                print(
                    f"Warning: File size {file_size} is less than expected {expected_total}."
                )
                # We can still proceed with what we have

            self.frames = []
            # read until the end of the file
            while True:
                header = f.read(24)
                if not header or len(header) < 24:
                    break

                energy, mag, r, c = struct.unpack("ddii", header)
                spins_raw = f.read(r * c)
                if not spins_raw:
                    break

                grid = np.frombuffer(spins_raw, dtype=np.int8).reshape(r, c)
                self.frames.append({"energy": energy, "mag": mag, "grid": grid})

        print(f"Successfully loaded {len(self.frames)} frames.")
        return True

    def animate(self):
        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 5))

        # Left: Grid
        img = ax1.imshow(self.frames[0]["grid"], cmap="binary", vmin=-1, vmax=1)

        stats_text = ax1.text(
            0.5,
            1.05,
            "",
            transform=ax1.transAxes,
            ha="center",
            fontsize=10,
            fontweight="bold",
            bbox=dict(facecolor="white", alpha=0.9, edgecolor="black"),
        )

        # Right: Energy History
        energies = [f["energy"] for f in self.frames]
        (line,) = ax2.plot([], [], "r-")
        ax2.set_xlim(0, len(self.frames))
        ax2.set_ylim(min(energies), max(energies))
        ax2.set_title("Energy History")
        ax2.set_xlabel("Frame")

        def update(i):
            frame = self.frames[i]

            # Update Pixels
            img.set_data(frame["grid"])
            stats_text.set_text(
                f"Step: {i} | E: {frame['energy']:.2f} | M: {frame['mag']:.4f}"
            )
            # Update line graph up to current frame
            x_data = list(range(i))
            y_data = [f["energy"] for f in self.frames[:i]]
            line.set_data(x_data, y_data)

            return [img, stats_text, line]

        ani = FuncAnimation(
            fig, update, frames=len(self.frames), interval=20, blit=True
        )
        plt.tight_layout()
        plt.show()

    def run_sim(self, binary_path="./build/ising_sim"):
        if not os.path.exists(binary_path):
            print(f"!!Error!! cannot find binary at {binary_path}.  Did you build?")
            return False

        print(f"starting simulation with {self.frames} frames")
        args = [binary_path, str(self.frames)]

        result = subprocess.run(args, capture_output=True, text=True)

        if result.returncode == 0:
            print("Simulation successful")
            print(result.stdout)
        else:
            print("Simulation failed!")
            print(result.stderr)
            return False
