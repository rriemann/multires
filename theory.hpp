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
    const real dx;

public:
    theory_t(const size_t level = g_level) :
        N(1 << level)
      , dx(g_span[dimX]/N)
    {
    }

    real at(const location_t center, const real time) const {
        location_t tmp = center;
        tmp[0] = inDomain(center[0]-std::fmod(time*g_velocity, g_span[dimX]));
        assert(tmp[0] >= g_x0[dimX] && tmp[0] <= g_x1[dimX]);
        return g_f_eval(tmp);
    }

    real at(const real center, const real time) const {
        location_t tmp = {{center}};
        return at(tmp, time);
    }

    real at(const index_t index, const real time) const {
        return at(g_x0[dimX]+dx*index[dimX], time);
    }

};


#endif // THEORY_HPP
