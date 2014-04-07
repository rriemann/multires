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

monores_grid_t::monores_grid_t(const u_char level_max) :
    grid_t()
  , N(1 << level_max)
  , N2(N*N)
  , dx({{g_span[dimX]/N, g_span[dimY]/N}})
  , pointvector(std::vector<point_t>(N2))
{
    // find smallest dt
    real dt_x = g_cfl*dx[dimX]/g_velocity;
    real dt_y = g_cfl*dx[dimY]/g_velocity;
    dt = (dt_x < dt_y) ? dt_x : dt_y;

    #pragma omp parallel for collapse(2)
    for (size_t j = 0; j < N; ++j) { // y-direction
        for (size_t i = 0; i < N; ++i) { // x-direction
            assert(g_dimension == 2);
            index_t index({{i, j}});
            const point_t point(index, level_max, s_f_eval);
            pointvector[N*j+i] = point;
        }
    }

    point_t *last = 0;
    for (size_t i = 0; i < N2; ++i) {
        point_t *p = &pointvector[i];
        p->m_next = last;
        last = p;
    }
}

void monores_grid_t::timeStepDirection(bool directionX)
{
    if (directionX) {
        // direction X

        // update inner cell values
        #pragma omp parallel for collapse(2)
        for (size_t j = 0; j < N; ++j) { // y-direction (full range)
            for (size_t i = 1; i < N-1; ++i) { // x-direction (range w/o edges)
                const size_t o = j*N; // offset
                pointvector[o+i].m_flow = flowHelper(
                            pointvector[o+i].m_phi,
                        pointvector[o+i-1].m_phi,
                        pointvector[o+i+1].m_phi,
                        dx[dimX], dt);
            }
        }

        // deal with edges
        #pragma omp parallel for
        for (size_t i = 0; i < N; ++i) { // y-direction
            // edge x = 0
            pointvector[i*N].m_flow = flowHelper(
                        pointvector[i*N    ].m_phi,
                    pointvector[i*N+N-1].m_phi,
                    pointvector[i*N+1  ].m_phi,
                    dx[dimX], dt);
            // edge x = N-1
            pointvector[i*N+N-1].m_flow = flowHelper(
                        pointvector[i*N+N-1].m_phi,
                    pointvector[i*N+N-2].m_phi,
                    pointvector[i*N    ].m_phi,
                    dx[dimX], dt);
        }

        // timestep
        #pragma omp parallel for collapse(2)
        for (size_t j = 0; j < N; ++j) { // y-direction (full range)
            for (size_t i = 1; i < N; ++i) { // x-direction (range w/o edges)
                const size_t o = j*N; // offset
                pointvector[o+i].m_phi += timeStepHelperFlow(
                            pointvector[o+i].m_flow,
                        pointvector[o+i-1].m_flow,
                        dx[dimX], dt);
            }
        }

        // deal with edges
        #pragma omp parallel for
        for (size_t i = 0; i < N; ++i) { // y-direction
            // edge x = 0
            pointvector[i*N].m_phi += timeStepHelperFlow(
                        pointvector[i*N].m_flow,
                    pointvector[i*N+N-1].m_flow,
                    dx[dimX], dt);
        }
    } else {
        // direction Y

        // update inner cell values
        #pragma omp parallel for collapse(2)
        for (size_t j = 1; j < N-1; ++j) { // y-direction (range w/o edges)
            for (size_t i = 0; i < N; ++i) { // x-direction (full range)
                const size_t o = j*N; // offset
                pointvector[o+i].m_flow = flowHelper(
                            pointvector[o+i].m_phi,
                            pointvector[o+i-N].m_phi,
                            pointvector[o+i+N].m_phi,
                            dx[dimY], dt);
            }
        }

        // deal with edges
        #pragma omp parallel for
        for (size_t i = 0; i < N; ++i) { // x-direction
            // edge y = 0
            pointvector[i].m_flow = flowHelper(
                        pointvector[i    ].m_phi,
                        pointvector[i+N2-N].m_phi,
                        pointvector[i+N].m_phi,
                        dx[dimY], dt);
            // edge y = N-1
            pointvector[N2-N+i].m_flow = flowHelper(
                        pointvector[N2-N+i].m_phi,
                        pointvector[N2-2*N+i].m_phi,
                        pointvector[i].m_phi,
                        dx[dimY], dt);
        }

        // timestep
        #pragma omp parallel for collapse(2)
        for (size_t j = 1; j < N; ++j) { // y-direction (range w/o edges)
            for (size_t i = 0; i < N; ++i) { // x-direction (full range)
                const size_t o = j*N; // offset
                pointvector[o+i].m_phi += timeStepHelperFlow(
                            pointvector[o+i].m_flow,
                            pointvector[o+i-N].m_flow,
                            dx[dimY], dt);
            }
        }

        // deal with edges
        #pragma omp parallel for
        for (size_t i = 0; i < N; ++i) { // x-direction
            // edge y = 0
            pointvector[i].m_phi += timeStepHelperFlow(
                        pointvector[i    ].m_flow,
                        pointvector[i+N2-N].m_flow,
                        dx[dimY], dt);
        }
    }
}

real monores_grid_t::timeStep()
{
    static u_short counter = 0;
    bool flip = counter % 2 == 0;
    timeStepDirection(flip);
    timeStepDirection(!flip);
    ++counter;

    m_time += dt;
    return dt;
}

grid_t::iterator monores_grid_t::begin()
{
    return iterator(&pointvector[N2-1]);
}

grid_t::iterator monores_grid_t::end()
{
    return iterator();
}
