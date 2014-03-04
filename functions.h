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

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "settings.h"

#include <cmath>
#include <algorithm>


inline real f_eval_gauss(location_t x) {
    const real x_shift = pow(x[dimX]-0.5,2)
                       + pow(x[dimY]-0.5,2);

    return exp(-200*x_shift*x_shift);
}

inline real f_eval_linear(location_t x) {
    return (x[dimX]+x[dimY])/2;
}

inline real f_eval_square(location_t x) {
    return (1-4*pow(x[dimX]-0.5,2))*(1-4*pow(x[dimY]-0.5,2));
}

const field_generator_t g_f_eval = f_eval_gauss;
// const field_generator_t g_f_eval = f_eval_square;

#endif // FUNCTIONS_H
