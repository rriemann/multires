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

#ifndef THEORY_BASE_HPP
#define THEORY_BASE_HPP

#include "settings.h"
#include <iostream>

class theory_base;

class theory_base
{

private:
    const propertyGenerator_t m_propertyGenerator;
    // const size_t level;
    const size_t N;
    const real dx;
    // const real dt;

public:
    theory_base(const propertyGenerator_t &propertyGenerator, const size_t level = g_level) :
        m_propertyGenerator(propertyGenerator)
      // , level(level)
      , N(level2N(level))
      , dx(g_span/N)
      // , dt(g_cfl*dx/g_velocity)
    {

    }

    real at(const realarray center, const real time) const {
        realarray tmp = center;
        tmp[0] = inDomain(center[0]-std::fmod(time*g_velocity, g_span));
        assert(tmp[0] >= x0 && tmp[0] <= x1);
        return m_propertyGenerator(tmp);
    }

    real at(const real center, const real time) const {
        realarray tmp {{center}};
        return at(tmp, time);
    }

    real at(const size_t i, const real time) const {
        // FIXME why minus? probably due to inDomain?
        return at(x0+dx*i, time);
    }

    typedef theory_base* theory_p;

};
typedef theory_base theory_t;


#endif // THEORY_BASE_HPP
