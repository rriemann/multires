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

#include "regular_base.hpp"

regular_base::regular_base(const propertyGenerator_t &propertyGenerator, const size_t level, const boundaryCondition_t boundaryCondition) :
    m_propertyGenerator(propertyGenerator)
  , m_boundaryCondition(boundaryCondition)
  , N((2 << level) + 1)
  , dx(g_span/N)
#ifndef BURGER
  , dt(g_cfl*dx/g_velocity)
#else
  , dt(g_cfl*dx/20) // assuming max(velocity = data[j]) <= 20
#endif
  , m_time(0)
{
    data.reserve(N);
    data2.reserve(N);
    xvalues.reserve(N);

    // std::cerr << "regular dt: " << dt << std::endl;

    for(size_t i = 0; i < N; ++i) {
        real x = i*dx+x0;
        xvalues.push_back(x);
        realarray center;
        center[dimX] = x;
        data.push_back(m_propertyGenerator(center));
    }
}

real regular_base::timeStep()
{
    // update temporary data;
    data2 = data;

    // update inner cell values
    for(size_t j = 1; j < N-1; ++j){
        data[j] = timeStepHelper(data2[j], data2[j-1], data2[j+1], dx, dt);
    }

    // deal with boundaries
    if(m_boundaryCondition == bcPeriodic) {
        data[0]   = timeStepHelper(data2[0  ], data2[N-1], data2[1], dx, dt);
        data[N-1] = timeStepHelper(data2[N-1], data2[N-2], data2[0], dx, dt);
    } else abort();

    m_time += dt;
    return dt;
}

regular_base::regular_u regular_base::c_root = regular_u(nullptr);
