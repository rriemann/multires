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
    // generation of childrens, e.g.: only root = 0, grand-children = 2
    // total number of nodes, including (childsbyDimension) boundary elements

    real simulationTime = g_span[dimX]/g_velocity; // 50 periods

#ifdef REGULAR
    monores_grid_t grid(g_level);
#else
    multires_grid_t grid(g_level);
#endif

    std::cerr << "\nsize: " << grid.size() << std::endl;

    // output file
    std::ofstream file("/tmp/output.txt");

    auto start = std::chrono::steady_clock::now();

    do {
        static int c = 0;
        grid.timeStep();
        if(c++ % 200 == 0) {
            for(const point_t point: grid) {
                // std::cerr << point.m_x[dimX] << " : " << point.m_phi << std::endl;
                file << boost::format("%e %e %e\n")
                        % point.m_x[dimX]
                        % point.m_phi
                        % grid.getTime();
            }
            file << std::endl << std::endl << std::endl;
        }
    } while(grid.getTime() < simulationTime);

    auto done = std::chrono::steady_clock::now();

    std::cerr << "simulation time passed: " << grid.getTime() << std::endl;
    double elapsed_time = std::chrono::duration_cast<std::chrono::duration<double>>(done - start).count();
    std::cerr << "calculation time: " << elapsed_time << std::endl;

    size_t size = grid.size();
    std::cerr << "used nodes: " << size << "/" << (1 << g_level) << "=" << real(size)/(1 << g_level) << std::endl;


    file.close();
    std::cerr << "try: gnuplot -p -e \"plot '/tmp/output.txt' using 1:2\"" << std::endl;

    return 0;
}
