from sim_wrapper import grid_helper, scripts


def main():
    grid_helper.generate_config(64, 64)

    sim = scripts.Simulation(frames=500)

    sim.run_sim()

    if sim.get_data():
        sim.animate()


if __name__ == "__main__":
    main()
