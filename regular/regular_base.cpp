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
  , dt(g_cfl*dx/g_velocity)
  , alpha(g_velocity*dt/dx) // in this case: alpha = g_clf
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

    // http://www.exp.univie.ac.at/cp1/cp1-6/node72.html (closed form)
    // time step with Lax-Wendroff method
    for(size_t j = 1; j < N-1; ++j){
        data[j]   = data2[j]   - alpha/2*(data2[j+1]-data2[j-1]-alpha*(data2[j+1]-2*data2[j]+data2[j-1]));
    }

    // deal with boundaries
    if(m_boundaryCondition == bcPeriodic) {
        data[0]   = data2[0]   - alpha/2*(data2[1]-data2[N-1]-alpha*(data2[1]-2*data2[0]  +data2[N-1]));
        data[N-1] = data2[N-1] - alpha/2*(data2[0]-data2[N-2]-alpha*(data2[0]-2*data2[N-1]+data2[N-2]));
    } else abort();

    m_time += dt;
    return dt;
}

regular_base::regular_u regular_base::c_root = regular_u(nullptr);
