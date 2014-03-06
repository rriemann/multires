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

#include <assert.h>

#include "monores_grid.hpp"
#include "functions.h"

monores_grid_t::monores_grid_t(const u_char level_max) :
    grid_t()
  , N(1 << level_max)
  , dx(g_span[dimX]/N)
  , dt(g_cfl*dx/g_velocity)
{
    pointvector.reserve(N);

    for(size_t i = 0; i < N; ++i) {
        assert(g_dimension == 1);
        index_t index({{i}});
        const point_t point(index, level_max, g_f_eval);
        pointvector.push_back(point);
    }
}

real monores_grid_t::timeStep()
{
    // update inner cell values
    for(size_t i = 1; i < pointvector.size()-1; ++i){
        pointvector[i].m_flow = flowHelper(pointvector[i].m_phi,
                                              pointvector[i-1].m_phi,
                                              pointvector[i+1].m_phi,
                                              dx, dt);
    }

    // deal with boundaries
    pointvector[0].m_flow   = flowHelper(pointvector[0  ].m_phi,
                                            pointvector[N-1].m_phi,
                                            pointvector[1].m_phi,
                                            dx, dt);
    pointvector[N-1].m_flow = flowHelper(pointvector[N-1].m_phi,
                                            pointvector[N-2].m_phi,
                                            pointvector[0].m_phi,
                                            dx, dt);

    // time step
    for(size_t i = 1; i < pointvector.size()-1; ++i){
        pointvector[i].m_phi = pointvector[i].m_phi - (dt/dx)*g_velocity*(pointvector[i].m_flow - pointvector[i-1].m_flow);
    }

    // deal with boundaries
    pointvector[0].m_phi = pointvector[0].m_phi - (dt/dx)*g_velocity*(pointvector[0].m_flow - pointvector[N-1].m_flow);
    pointvector[N-1].m_phi = pointvector[N-1].m_phi - (dt/dx)*g_velocity*(pointvector[N-1].m_flow - pointvector[N-2].m_flow);

    m_time += dt;
    return dt;
}

std::vector<point_t>::iterator monores_grid_t::begin()
{
    return pointvector.begin();
}

std::vector<point_t>::iterator monores_grid_t::end()
{
    return pointvector.end();
}
