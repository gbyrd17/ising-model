#pragma once
#include "./lattice.hpp"
#include <omp.h>
#include <random>

class Simulator {
private:
  std::uniform_real_distribution<double> dist{0.0, 1.0};

  /* Pointer: Simulator::&get_rng(); (private)
   * Returns: (std::mt19937); Allows for cpu multithreading in probability
   *    calls during update_lattice() method.
   */
  std::mt19937 &get_rng() {
    // Each thread gets its own static instance, seeded by thread ID
    thread_local std::mt19937 engine(std::random_device{}() ^
                                     omp_get_thread_num());
    return engine;
  }

  const std::vector<int> init_site;

  double get_energy_diff(int x, int y);

  void find_magnitization();
  void find_total_energy();

  void write_bin();

  Lattice grid;
  double current_energy;
  double current_mag;

public:
  Simulator(Lattice lattice);

  void get_site();
  void try_flip();
  void update_lattice();
  // TODO: IMPLEMENT finalize()
  void finalize();
};
