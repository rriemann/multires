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

#ifndef REGULAR_BASE_HPP
#define REGULAR_BASE_HPP

#include "settings.h"

#include <iostream>
#include <memory>
#include <cassert>
#include <boost/format.hpp>

struct regular_base;

struct regular_base
{

    typedef regular_base* regular_p;
    typedef std::unique_ptr<regular_base> regular_u;

    static regular_p createRoot(const propertyGenerator_t &propertyGenerator, const boundaryCondition_t boundaryCondition = bcPeriodic)
    { c_root = regular_u(new regular_base(propertyGenerator, boundaryCondition)); return c_root.get(); }

    real getTime() const
    { return m_time; }

    real operator[](size_t i) const
    { return data[i]; }

    real timeStep();

    ~regular_base();

private:
    regular_base(const regular_base&) = delete; // remove copy constructor
    regular_base(const propertyGenerator_t &propertyGenerator, const boundaryCondition_t boundaryCondition = bcPeriodic);

    const propertyGenerator_t m_propertyGenerator;
    const boundaryCondition_t m_boundaryCondition;
    static regular_u c_root;
    const size_t N;
    const real dx;
    const real dt;
    const real alpha;
    real m_time;

    real propertyGeneratorWrapper(real x) const
    { realarray center; center[dimX] = x; return m_propertyGenerator(center); }

    realvector data, data2, xvalues;
};

typedef regular_base::regular_p regular_tp;
typedef regular_base regular_t;

#endif // REGULAR_BASE_HPP
