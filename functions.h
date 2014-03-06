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


inline real f_eval_gauss(location_t x) {
    const real x_shift = x[dimX]-0.5;

    return exp(-30*x_shift*x_shift);
}

inline real f_eval_hat(location_t x) {
    const real xx = x[dimX];
    if (std::fabs(xx-0.5) < 0.25) {
        return 1;
    } else {
        return 0;
    }
}

const field_generator_t g_f_eval = f_eval_gauss;

#endif // FUNCTIONS_H
