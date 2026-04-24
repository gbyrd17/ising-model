import matplotlib.pyplot as plt

from scripts import process_data


def show_sim():
    data = process_data.read_bin()
    if data:
        print(f"Total Energy:\t\t{data['energy']:.3e}")
        print(f"Total Magnetization:\t{data['magnetization']:.3e}")
        print(f"Grid Shape:\t\t{data['grid'].shape}\n")

    plt.imshow(data["grid"], cmap="binary")
    plt.title("Ising Model Configuration")
    plt.show()
