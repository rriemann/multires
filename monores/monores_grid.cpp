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
        const location_t location = {{i*dx+g_x0[0]}};
        const point_t point(location, f_eval(location));
        pointvector.push_back(point);
    }
}

real monores_grid_t::timeStep()
{
    // update temporary data;
    for(point_t &point: pointvector) {
        point.m_phiBackup = point.m_phi;
    }

    // update inner cell values
    for(size_t i = 1; i < pointvector.size()-1; ++i){
        pointvector[i].m_phi = timeStepHelper(pointvector[i].m_phiBackup, pointvector[i-1].m_phiBackup, pointvector[i+1].m_phiBackup, dx, dt);
    }

    // deal with boundaries
    pointvector[0].m_phi   = timeStepHelper(pointvector[0  ].m_phiBackup, pointvector[N-1].m_phiBackup, pointvector[1].m_phiBackup, dx, dt);
    pointvector[N-1].m_phi = timeStepHelper(pointvector[N-1].m_phiBackup, pointvector[N-2].m_phiBackup, pointvector[0].m_phiBackup, dx, dt);

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
