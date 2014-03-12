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

    \license GNU General Public License

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
const size_t g_level  = 7;

/*!
   \brief maschine accurancy for the chosen accurancy
   \sa typedef real
 */
const real g_eps = std::numeric_limits<real>::epsilon();

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
typedef std::function<real(location_t)> field_generator_t;

const real g_velocity = 0.5; //!< velocity used in the advection equation solver

/*!
   \brief calculates the flow difference
   \param flow next neighbour
   \param flow_left previous neighbougr
   \param dx grid size
   \param dt time step
   \return a flow difference to the actual value

   The flow difference to the actual value is calculated using the flow values
   perviously set by flowHelper().

   \sa flowHelper()
 */
inline real timeStepHelperFlow(const real &flow, const real &flow_left, const real &dx, const real &dt) {
    return - (dt/dx)*g_velocity*(flow - flow_left);
}

/*!
   \brief helper function to implement the minmod limiter
 */
inline real minmod(const real a, const real b)
{
    if (a*b > 0) {
        if (fabs(a) < fabs(b)){
            return a;
        } else {
            return b;
        }
    } else {
        return 0;
    }
}

/*!
   \brief calculates the flow through the interfaces of the nodes
   \param ee flow of this node
   \param el flow of its previous neighbour
   \param er flow of its next neighbour
   \param dx node size
   \param dt time step
   \return flow

   With this function the flux at the interfaces is calculated. It is later
   on used to do the actual time step using timeStepHelperFlow()

    LIMITER can be defined to enable the limiting of the derivates and get smoothed
    behavior close to shocks.

   \sa timeStepHelperFlow()
 */
inline real flowHelper(const real &ee, const real &el, const real &er,
                            const real &dx, const real &dt)
{
#ifdef LIMITER
   const real derivative = minmod(ee - el, er - ee);
#else
   const real derivative = (er-el)/2;
#endif
   real a_L = ee + 0.5*(1-dt/dx*g_velocity)*derivative;
   return a_L;
}

#endif // SETTINGS_H
