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

#include "grid.hpp"
#include "functions.h"

grid_t::grid_t()
{
}

size_t grid_t::size()
{
    return std::distance(begin(), end());
}

real grid_t::absL2Error()
{
    //------- ABSOLUTE NUMERICAL ERROR (L2)-------
    real esum = 0; //!<  Counter for the error calculation.
    // #pragma omp parallel for reduction(+:esum) // TODO
    for (point_t &p: *this) {
    // for (iterator it = begin(); it != end(); ++it) {
        // point_t &p = *it;
        field_t U;
        g_f_eval(p.m_x, getTime(), U, 0);
        esum += pow(p.m_U[dimX]-U[dimX],2)+pow(p.m_U[dimY]-U[dimY],2);
    }

    //-------OUTPUT----------
    // calculate absolute error in L^2 norm and output
    return sqrt(esum/size());
}
