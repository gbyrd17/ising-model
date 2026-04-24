#pragma once
#include "./lattice.hpp"
#include <omp.h>
#include <random>
#include <vector>

class Simulator {
private:
  Lattice grid;
  std::uniform_real_distribution<double> dist{0.0, 1.0};

  std::uniform_int_distribution<int> x_dist;
  std::uniform_int_distribution<int> y_dist;

  std::random_device rd;
  std::mt19937 gen{rd()};

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

  double get_energy_diff(int x, int y);

  void find_magnitization();
  void find_total_energy();

  double current_energy;
  double current_mag;

  bool first_pass;

public:
  Simulator(Lattice lattice);

  std::vector<int> size;
  std::vector<int> site;

  void try_flip();
  void update_site();
  void write_bin();
};
