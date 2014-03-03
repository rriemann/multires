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

class theory_t
{

private:
    const size_t N;
    const location_t dx;

public:
    theory_t(const size_t level = g_level) :
        N(1 << level)
      , dx({{g_span[dimX]/N, g_span[dimY]/N}})
    {
    }

    real at(const location_t center, const real time) const {
        location_t tmp = center;
        for (u_char dim = 0; dim < g_dimension; ++dim) {
            real x = fmod(tmp[dim] - time*g_velocity, g_span[dim]);
            tmp[dim] = fmod(fabs(x - g_x0[dim] + g_span[dim]), g_span[dim]) + g_x0[dim];
            assert(tmp[dim] >= g_x0[dim] && tmp[dim] <= g_x1[dim]);
        }

        const real value = g_f_eval(tmp);
        assert(value > -2 && value < 2);
        return value;
    }

    real at(const index_t &index, const real time) const {
        location_t center;
        for (u_char dim = 0; dim < g_dimension; ++dim) {
            center[dim] = g_x0[dim]+dx[dim]*index[dim];
        }
        return at(center, time);
    }

};


#endif // THEORY_HPP
