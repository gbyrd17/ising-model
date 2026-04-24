#include "../include/ising/simulator.hpp"
#include "../include/ising/lattice.hpp"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <omp.h>
#include <random>
#include <stdexcept>
#include <vector>

/* Constructor: Simulator::Simulator(Lattice grid); (public)
 * Given a pregenerated Lattice (var name: grid), pre-select
 *    a site at random to use in try_flip initialization method
 *
 */
Simulator::Simulator(Lattice lattice) : grid(std::move(lattice)) {
  size = this->grid.get_size();

  std::uniform_int_distribution<int> x_dist{0, size[0] - 1};
  std::uniform_int_distribution<int> y_dist{0, size[1] - 1};
  std::random_device rd;
  std::mt19937 gen(rd());

  init_site = {x_dist(gen), y_dist(gen)};
  std::cout << "Simulator initialized by flipping site at: (" << init_site[0]
            << ", " << init_site[1] << ")." << std::endl;
  return;
};

/* Method:  Simulator::get_energy_diff(int x, int y); (private)
 * Returns: (double); determines total energy diff between site at
 *    idx(x, y) and surrounding sites.  Used in Metropolis-Hastings
 *    to determine whether or not to flip spin of site.
 *
 * Equation => E = 2 * sum_(ij) [coupling_(ij) * spin_(i) * spin_(j)]
 *    where: i : idx(x, y) = i and j : idx(x', y') = j with x', y'
 *    belonging to the set of sites neighboring site x, y.  That is,
 *    j are all the sites neighboring i.
 */
double Simulator::get_energy_diff(int x, int y) {
  int idx = grid.get_index(x, y);
  int8_t spin_idx = grid.get_spin(idx);

  size_t left_neighbor = grid.get_index(x - 1, y);
  size_t down_neighbor = grid.get_index(x, y - 1);
  double interaction =
      grid.x_coupling[idx] * grid.get_spin(grid.get_index(x + 1, y)) +
      grid.x_coupling[left_neighbor] * grid.get_spin(left_neighbor) +
      grid.y_coupling[idx] * grid.get_spin(grid.get_index(x, y + 1)) +
      grid.y_coupling[down_neighbor] * grid.get_spin(down_neighbor);

  return 2.0f * spin_idx * (interaction + grid.field[idx]);
};

/* Method: Simulator::find_magnitization(); (private)
 * Returns: (void); determines internal magnitization of simulation in
 *    initial state.
 *
 *    Updates Simulator.current_mag (private).
 *
 * Equation => M = (1/N) sum_(i=1)^(N) spin_(i)
 */
void Simulator::find_magnitization() {
  for (int x = 0; x < size[0]; ++x) {
    for (int y = 0; y < size[1]; ++y) {
      int idx = grid.get_index(x, y);
      this->current_mag += grid.get_spin(idx);
    }
  }
  this->current_mag = this->current_mag / grid.total_sites();
  return;
}

/* Method: Simulator::find_total_energy(); (private)
 * Returns: (void); determines total energy of simulation in initial
 *    state.
 *
 *    Updates Simulator.current_energy (private).
 *
 * Equation => E = sum_(ij) [coupling_(ij) * spin_(i) * spin_(j)]
 */
void Simulator::find_total_energy() {
  this->current_energy = 0;
  for (int x = 0; x < size[0]; ++x) {
    for (int y = 0; y < size[1]; ++y) {
      int idx = grid.get_index(x, y);

      double interaction =
          grid.x_coupling[idx] * grid.get_spin(grid.get_index(x, y + 1)) +
          grid.y_coupling[idx] * grid.get_spin(grid.get_index(x + 1, y));

      double mag_term = grid.field[idx] * grid.get_spin(idx);

      this->current_energy -= (interaction + mag_term);
    }
  }
  return;
}

/* Method: Simulator::write_bin(); (private)
 * Returns: (void); Records current total energy, magnitization,
 *    and spin configuration to a .bin file.  File readable by
 *    python frontend.  Used by python frontend to generate
 *    visualization and output for user.
 */
void Simulator::write_bin() {
  const std::string &filename = "output.bin";
  std::ofstream outFile(filename, std::ios::binary | std::ios::app);

  if (!outFile.is_open()) {
    throw std::runtime_error("Failed to open/initialize output file");
  }

  outFile.write(reinterpret_cast<const char *>(&this->current_energy),
                sizeof(double));
  outFile.write(reinterpret_cast<const char *>(&this->current_mag),
                sizeof(double));

  auto *spin_data = grid.spin_pointer();
  outFile.write(reinterpret_cast<const char *>(spin_data),
                grid.total_sites() * sizeof(int8_t));

  outFile.close();
  std::cout << "Simulation results written to: " << &filename << "."
            << std::endl;
}

/* Method: Simulator::try_flip(); (public)
 * Returns: (void); Uses pre-initialized starting site (see constructor) as
 *    candidate for first spin-flip.  Decides when to flip based on
 *    Metropolis-Hastings algorithm:  If energy diff between neighbor sites is
 *    <= 0, spin at site is flipped.  If not, checks to see if the flip is
 *    thermodynamically favorable using probabalistic stat. mech.
 *
 *    Updates (Lattice) grid by flipping spin at site (idx).
 *
 * Equation => P = exp( -dE / T )
 *    where; dE is change in energy between neighboring sites (always > 0
 *    in the case where this is called), T is temperature.  Probability is
 *    according to Boltzmann distribution.
 */
void Simulator::try_flip() {
  int idx = grid.get_index(init_site[0], init_site[1]);

  double dE = get_energy_diff(init_site[0], init_site[1]);
  if (dE <= 0) {
    grid.flip_spin(idx);
  } else {
    double probabilty = std::exp(-dE / grid.get_temp());
    if (dist(get_rng()) < probabilty) {
      grid.flip_spin(idx);
    }
  }
};

/* Method: Simulator::update_lattice(); (public)
 * Returns: (void);  Propagates Metropolis-Hastings algorithm across whole grid.
 *    Used to update grid after a change has been made; a site was flipped,
 * temperature modified, or coupling modified.
 *
 *    Updates (Lattice) grid by flipping sites according to Metropolis-Hastings.
 */
void Simulator::update_lattice() {
#pragma omp parallel for collapse(2)
  for (int x = 0; x < size[0]; ++x) {
    for (int y = 0; y < size[1]; ++y) {
      double dE = get_energy_diff(x, y);

      if (dE <= 0 || dist(get_rng()) < std::exp(-dE / grid.get_temp())) {
        grid.flip_spin(grid.get_index(x, y));
      }
    }
  }
  find_magnitization();
  find_total_energy();
};
