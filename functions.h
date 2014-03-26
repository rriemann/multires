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
/*! \file functions.h

    \author Robert Riemann (robert@riemann.cc)

    \brief initalizer definitions
 */


#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "settings.h"

#include <cmath>

inline void getTheory(const location_t &pos, const real &time, field_t &value, real *rho) {
    using namespace g_lb;
    const real &x = pos[dimX];
    const real &y = pos[dimY];
    value[dimX] = -g_lb::Ulat*cos(Kx*x)*sin(Kx*y)*exp(-2*Kx*Kx*vlat*time*Cl2);
    value[dimY] = +g_lb::Ulat*sin(Kx*x)*cos(Kx*y)*exp(-2*Kx*Kx*vlat*time*Cl2);
    if (rho) {
        real P = Ro*CsSquare-0.25*Ulat*Ulat*(cos(2*Kx*x)+cos(2*Kx*y)); //!< Pressure defined in the computational space.
        *rho = P/CsSquare;
    }
}

const field_generator_t g_f_eval = getTheory; //!< default initializer
// const field_generator_t g_f_eval = f_eval_square;

#endif // FUNCTIONS_H
