#include "../include/ising/lattice.hpp"
#include "../include/ising/simulator.hpp"
#include <nlohmann/json_fwd.hpp>

int main(int argc, char *argv[]) {
  int frames = (argc > 1) ? std::stoi(argv[1]) : 100;

  std::ifstream f("config.json");
  const nlohmann::json config = nlohmann::json::parse(f);
  Lattice grid(config);
  Simulator sim(grid);

  int attempts_per_frame = grid.total_sites();

  for (int f = 0; f < frames; ++f) {
    for (int i = 0; i < attempts_per_frame; ++i) {
      sim.update_site();
      sim.try_flip();
    }

    sim.write_bin();
  }
  return 0;
}
