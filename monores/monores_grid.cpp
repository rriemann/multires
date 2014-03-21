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
  , dt(1.0)
  , pointvector(std::vector<point_t>(N2))
{

    #pragma omp parallel for collapse(2)
    for (size_t j = 0; j < N; ++j) { // y-direction
        for (size_t i = 0; i < N; ++i) { // x-direction
            assert(g_dimension == 2);
            index_t index({{i, j}});
            point_t point(index, level_max, g_f_eval);
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

real monores_grid_t::timeStep()
{
    using namespace g_lb;

    for (u_char k = 0; k < Nl; ++k) {
        //-------COLLISION-------
        #pragma omp parallel for
        for (size_t i = 0; i < N2; ++i) {
            pointvector[i].collision(k);
        }
        //-------STREAMING-------
        #pragma omp parallel for collapse(2)
        for (u_short i = 0; i < N; ++i) {
            for (u_short j = 0; j < N; ++j) {
                point_t &p = pointvector[i+N*j];

                // compute index
                u_short nj = (Ny+j-ey[k]) % Ny;
                u_short ni = (Nx+i-ex[k]) % Nx;
                point_t &np = pointvector[ni+N*nj];
                p.m_f[k] = np.m_fbak;
            }
        }
    }

    //-------CALCULATION OF THE MACROSCOPIC VARIABLES-------
    #pragma omp parallel for
    for (size_t i = 0; i < N2; ++i) {
        pointvector[i].derivateMacroVariables();
    }

    //-------CALCULATION OF THE EQUILIBRIUM DISTRIBUTION FUNCTION-------
    #pragma omp parallel for
    for (size_t i = 0; i < N2; ++i) {
        pointvector[i].equilibriumHelper();
    }

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

real monores_grid_t::absL2Error()
{

    //------- ABSOLUTE NUMERICAL ERROR (L2)-------
    real esum = 0; //!<  Counter for the error calculation.
    #pragma omp parallel for collapse(2) reduction(+:esum) private(Uexact,Vexact)
    for (u_short i = 0; i < N; ++i) {
        for (u_short j = 0; j < N; ++j) {
            point_t &p = pointvector[i+N*j];
            field_t U;
            g_f_eval(p.m_index, getTime(), U, 0);
            esum += pow(p.m_U[dimX]-U[dimX],2)+pow(p.m_U[dimY]-U[dimY],2);
        }
    }

    //-------OUTPUT----------
    // calculate absolute error in L^2 norm and output
    return sqrt(esum/N2);
}
