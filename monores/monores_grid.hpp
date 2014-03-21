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

#ifndef MONORES_GRID_HPP
#define MONORES_GRID_HPP

#include "settings.h"

#include "grid.hpp"
#include "point.hpp"

class monores_grid_t : public grid_t
{
public:
    /*!
       \brief constructs a mono resolution grid
       \param level_max determines the number of nodes in the computation area

       The accurancy can be tuned by chosing level_max that is used to compute
       the number of grid points `N = (1 << level_max)` per dimension.
     */
    monores_grid_t(const u_char level_max);

    virtual real timeStep(); // see docu in grid_t

    virtual size_t size()
    { return N2; }

    // getters for ranged for, see:
    // http://stackoverflow.com/questions/8164567/how-to-make-my-custom-type-to-work-with-range-based-for-loops
    virtual iterator begin();
    virtual iterator end();

    real absL2Error();

    virtual ~monores_grid_t() {}

private:
    monores_grid_t(const monores_grid_t&) = delete; // remove copy constructor



    const size_t N; //!< number of points per dimension
    const size_t N2; //!< number of total points assuming \ref g_dimension = 2
    const location_t dx; //!< grid size in all dimensions of every nodes of this grid
    const real dt; //!< time step with respect to \ref g_cfl

    std::vector<point_t> pointvector; //!< actual grid data in a 1D array
};

#endif // MONORES_GRID_HPP
