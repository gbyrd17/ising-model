#include "../include/ising/lattice.hpp"
#include "../include/ising/simulator.hpp"
#include <nlohmann/json_fwd.hpp>

int main() {
  std::ifstream f("config.json");
  const nlohmann::json config = nlohmann::json::parse(f);
  Lattice grid(config);
  Simulator sim(grid);

  sim.write_bin();
  return 0;
}
