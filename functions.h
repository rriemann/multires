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
#include <algorithm>


/*!
   \brief f_eval_gauss implements a gaussion curve in 2D
   \param x location in space
   \return field value
 */
inline real f_eval_gauss(location_t x) {
    const real x_shift = pow(x[dimX]-0.5,2)
                       + pow(x[dimY]-0.5,2);

    return exp(-200*x_shift*x_shift);
}

/*!
   \brief f_eval_square implements a square function
   \param x location in space
   \return field value
 */
inline real f_eval_square(location_t x) {
    return (1-4*pow(x[dimX]-0.5,2))*(1-4*pow(x[dimY]-0.5,2));
}

/*!
   \brief f_eval_hat implements a 1D hat function including to discontiunities
   \param x location in space
   \return field value
 */
inline real f_eval_hat(location_t x) {
    const real xx = x[dimX];
    if (std::fabs(xx-0.5) < 0.25) {
        return 1;
    } else {
        return 0;
    }
}


/*!
   \brief f_eval_box implements a 2-level box function, which is rotated by 30 degrees (can be changed in code)
   \param x location in space
   \return field value
 */
inline real f_eval_box(location_t x) {
    location_t tmp = x;
    real rad = 8*atan(1)*(30.0/360.0);
    x[dimX] = (tmp[dimX]-0.5)*cos(rad)+(tmp[dimY]-0.5)*sin(rad) + 0.5;
    x[dimY] = (tmp[dimY]-0.5)*cos(rad)-(tmp[dimX]-0.5)*sin(rad) + 0.5;

    if ((fabs(x[dimX]-0.5) < 0.125) && (fabs(x[dimY]-0.5) < 0.125)) {
        return 1;
    } else if ((fabs(x[dimX]-0.5) < 0.25) && (fabs(x[dimY]-0.5) < 0.25)) {
        return 0.5;
    } else {
        return 0;
    }
}

const field_generator_t g_f_eval = f_eval_gauss; //!< default initializer
// const field_generator_t g_f_eval = f_eval_square;

#endif // FUNCTIONS_H
