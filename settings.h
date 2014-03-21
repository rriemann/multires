/***************************************************************************************
 * Copyright (c) 2014 Robert Riemann <robert@riemann.cc>                                *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/
/*! \file settings.h

    \author Robert Riemann (robert@riemann.cc)

    \brief globals settings

    This header files defines the global settings that are used throughout
    the project.
 */
#ifndef SETTINGS_H
#define SETTINGS_H

#include <cstddef>
#include <cmath>
#include <array>
#include <vector>
#include <functional>
#include <limits>
#include <cassert>
#ifdef _OPENMP
#include <omp.h>
#endif

constexpr u_char g_dimension = 2; //!< number of dimensions of the grid
constexpr short  g_childs = (1 << g_dimension); //!< number of children per node
#ifdef _OPENMP
const u_char g_level_fork = log(omp_get_num_procs())/log(g_childs); //!< tree level until we create seperate threads for each child
#endif

typedef double real; //!< determines the accurancy of the computations, e.g. double or float precision
typedef std::array<real, g_dimension> location_t; //!< type to save a point in space
typedef std::array<real, g_dimension> field_t;
typedef std::vector<real> real_vector;
typedef std::array<real, g_childs> environment_t; //!< type to keep all children of a node
typedef std::array<size_t, g_dimension> index_t; //!< type to save a point in space by its indices

/*!
   \brief cut-off accurancy to drop nodes in the multi resolution tree

   In the multir resolution module, the difference between the interpolation value
   and the actual value is computed. If it is lower than g_epsilon the node might
   be dismissed.
 */
const real g_epsilon  = 4e-3;

/*!
   \brief default grid level to derivate the number of grid points per dimension

   The number of grid points is calculated by `N = (1 << g_level)` which gives you
   just the power go 2 to \ref g_level.
 */
const size_t g_level  = 5;

/*!
   \brief maschine accurancy for the chosen accurancy
   \sa typedef real
 */
const real g_eps = std::numeric_limits<real>::epsilon();

const real pi = 4*atan(1);

const real g_cfl  = 0.1; //!< constant in CFL (Courant, Friedrichs, Lewy) condition

const location_t g_x0 = {{0, 0}}; //!< lower left point of computational domain
const location_t g_x1 = {{1, 1}}; //!< upper right point of computational domain
const location_t g_span = {{g_x1[0] - g_x0[0],
                            g_x1[1] - g_x0[1]}}; //!< size of computational domain

/*!
   \brief enumerates the dimensions
 */
enum dimension_t {
      dimX = 0
    , dimY = 1
    , dimZ = 2
};

/*!
   \brief defines the interface of a field initializer function to map a \ref location_t type to a \ref real value
 */
typedef std::function<void(const index_t &index, const real &time, field_t &value, real *rho)> field_generator_t;

/*!
   Variables needed for Latice-Boltzmann-Solver
 */
namespace g_lb {
    const u_short Nx = (1 << g_level);
    const u_short Ny = Nx;
    const u_char  Nl = 9;
    const u_short Length = Nx-1; //!< Length of the square computational domain in lattice units.
    const real Re = 10.0; //!< Reynolds number.
    const real tau = 0.65; //!< Relaxation time.
    const real omega = 1.0/tau; //!< Relaxation frequency.
    const real vlat = (tau-0.5)/3; //!< Lattice kinematic viscosity.
    const real CsSquare = 1.0/3; //!< Square of the speed of sound in lattice units.
    const real Ulat = Re*vlat/Length; //!< Lattice characteristic velocity.
    const real Kx = 2*pi/Length; //!< Wavenumber in the x- and y-direction.
    // const real i2s = 1; //!< index*i2s = space position (index to space)
    const real Ro = 1.0; //!< Initial fluid density in lattice and physical units.


    const real weight[9] = {4.0/9, 1.0/9, 1.0/9, 1.0/9, 1.0/9, 1.0/36, 1.0/36, 1.0/36, 1.0/36}; //!< Weighting factors.
    const char ex[9] = {0, 1, -1, 0, 0, 1, -1, -1, 1}; //!< X-component of the particle velocity.
    const char ey[9] = {0, 0, 0, 1, -1, 1, 1, -1, -1}; //!< Y-component of the particle velocity.
}


#endif // SETTINGS_H
