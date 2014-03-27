/***************************************************************************************
 * Copyright (c) 2013 Robert Riemann <robert@riemann.cc>                                *
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

#ifndef THEORY_HPP
#define THEORY_HPP

#include "settings.h"
#include "functions.h"
#include <iostream>

/*!
   \brief The theory_t class is a helper class to calculate the theoretical solution at given points in time

   This class is initialized with a maximum level to determine the smallest node size.
   Afterwards the theoretical result of the advection equation at given time points
   can be calculated using the at() functions.

   This class is used to initialize the field at time `t = 0` and furthermore to
   facilitate the error analysis.
 */
class theory_t
{

private:
    const size_t N; //!< number of grid points per dimension
    const location_t dx; //!< node size in one dimension

public:
    /*!
       \brief theory_t constructs a helper object to calculate theoretical field values
       \param level is the finest level of the corresponding grid
     */
    theory_t(const size_t level = g_level) :
        N(1 << level)
      , dx({{g_span[dimX]/N, g_span[dimY]/N}})
    {
    }

    /*!
       \brief at gives the field value at the point in space center at a given time
       \param center position in space at which the solution is calculated
       \param time at which the solution is calculated
       \return field value
     */
    real at(const location_t center, const real time) const {
        field_t U;
        g_f_eval(center, time, U, 0);
        return sqrt(pow(U[dimX],2)+pow(U[dimY],2));
    }

    /*!
       \brief at gives the field value at the index descirbing a point in space at a given time
       \param index describing a position in space at which the solution is calculated
       \param time at which the solution is calculated
       \return field value
     */
    real at(const index_t &index, const real time) const {
        location_t center;
        for (u_char dim = 0; dim < g_dimension; ++dim) {
            center[dim] = g_x0[dim]+dx[dim]*index[dim];
        }
        return at(center, time);
    }

};


#endif // THEORY_HPP
