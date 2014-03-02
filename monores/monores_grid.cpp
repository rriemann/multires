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
  , N2(N*N)
  , dx({{g_span[dimX]/N, g_span[dimY]/N}})
{
    // find smallest dt
    real dt_x = g_cfl*dx[dimX]/g_velocity;
    real dt_y = g_cfl*dx[dimY]/g_velocity;
    dt = (dt_x < dt_y) ? dt_x : dt_y;

    pointvector.reserve(N2);

    for (size_t j = 0; j < N; ++j) { // y-direction
        for (size_t i = 0; i < N; ++i) { // x-direction
            assert(g_dimension == 2);
            index_t index({{i, j}});
            const point_t point(index, level_max, g_f_eval);
            pointvector.push_back(point);
        }
    }
}

real monores_grid_t::timeStep()
{
    // directional split Lax-Wendroff
    // speed = g_velocity/sqrt(2)*(1,1)
    // dt -> dt/sqrt(2)
    // const real dt2 = dt/sqrt(2);
    const real &dt2 = dt;

    // x-direction

    // update temporary data;
    for (point_t &point: pointvector) {
        point.m_phiBackup = point.m_phi;
    }

    // update inner cell values
    for (size_t j = 0; j < N; ++j) { // y-direction (full range)
        for (size_t i = 1; i < N-1; ++i) { // x-direction (range w/o edges)
            const size_t o = j*N; // offset
            pointvector[o+i].m_phi = timeStepHelper(
                        pointvector[o+i].m_phiBackup,
                        pointvector[o+i-1].m_phiBackup,
                        pointvector[o+i+1].m_phiBackup,
                        dx[dimX], dt2);
        }
    }

    // deal with edges
    for (size_t i = 0; i < N; ++i) { // y-direction
        // edge x = 0
        pointvector[i*N].m_phi = timeStepHelper(
                    pointvector[i*N    ].m_phiBackup,
                    pointvector[i*N+N-1].m_phiBackup,
                    pointvector[i*N+1  ].m_phiBackup,
                    dx[dimX], dt2);
        // edge x = N-1
        pointvector[i*N+N-1].m_phi = timeStepHelper(
                    pointvector[i*N+N-1].m_phiBackup,
                    pointvector[i*N+N-2].m_phiBackup,
                    pointvector[i*N    ].m_phiBackup,
                    dx[dimX], dt2);
    }

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
