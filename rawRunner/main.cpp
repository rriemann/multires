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

#include <string>
#include <memory>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <functional>
#include <chrono>
#include <boost/format.hpp>

// using namespace std;

#ifdef REGULAR
#include "monores/monores_grid.hpp"
#else
#include "point.hpp"
#include "multires/multires_grid.hpp"
#endif

#include "functions.h"

int main()
{
    #ifdef _OPENMP
    const u_char num_procs = omp_get_num_procs(); //!< number of available processors
    #else
    const u_char num_procs = 1;
    #endif
    std::cerr << "processors in use: " << short(num_procs) << std::endl;


    // generation of childrens, e.g.: only root = 0, grand-children = 2
    // total number of nodes, including (childsbyDimension) boundary elements

    real simulationTime = 10000;

#ifdef REGULAR
    monores_grid_t grid(g_level);
#else
    multires_grid_t grid(g_level);
#endif

    auto start = std::chrono::steady_clock::now();

    while (grid.getTime() < simulationTime) {
        grid.timeStep();
    };

    auto done = std::chrono::steady_clock::now();

    std::cerr << "simulation time passed: " << grid.getTime() << std::endl;
    double elapsed_time = std::chrono::duration_cast<std::chrono::duration<double>>(done - start).count();
    std::cerr << "calculation time: " << elapsed_time << std::endl;

    size_t size = grid.size();
    size_t NN = pow(1 << g_level, g_dimension);
    std::cerr << "used nodes: " << size << "/" << NN << "=" << real(size)/NN << std::endl;


    // output file
#ifndef REGULAR
    grid.unfold(g_level);
#endif
    std::cerr << "after unfold: size = " << grid.size() << std::endl;
    std::ofstream file("/tmp/output.txt");
    file << "# x y Ux Uy" << std::endl;
    for(const point_t point: grid) {
        // std::cerr << point.m_x[dimX] << " : " << point.m_phi << std::endl;
        /*
        */
        file << boost::format("%e %e %e %e\n")
                % point.m_x[dimX]
                % point.m_x[dimY]
                // % point.m_index[dimX]
                // % point.m_index[dimY]
                % point.m_U[dimX]
                % point.m_U[dimY];
        /*
        file << boost::format("%e ") % point.m_phi;
        static size_t count = 0;
        const size_t N = (1 << g_level);
        if (++count % N == 0 ) file << std::endl;
        */
    }
    file.close();

    std::cerr << "error: " << (boost::format(" %1.20e") % grid.absL2Error()) << std::endl;
    // assert(fabs(0.00087126772875501965962-AbsL2error) <= eps); // Tsim = 20, Nx = 21
    // assert(fabs(0.000557275730831370335813-AbsL2error) <= eps); // Tsim = 1, Nx = 21

    return 0;
}
