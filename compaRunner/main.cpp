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

// using namespace std;

#include "multires/multires_grid.hpp"
#include "monores/monores_grid.hpp"
#include "theory.hpp"

#include "functions.h"


int main()
{
    ///////////// CONFIG //////////////////////
#define NORM_L_INF // uncomment to use L_1 norm

    // real simulationTime = g_span[dimX]/g_velocity*5; // 1 period
    size_t loops_max = 20;

    std::array<real,7> steps_level;
    for(size_t i = 0; i < steps_level.size(); ++i) {
         steps_level[i] = 3+i;
    }

    std::array<real,8> steps_epsilon;
    for(size_t i = 0; i < steps_epsilon.size(); ++i) {
         steps_epsilon[i] = 0.00001*pow(2,0.5*i);
    }

    /*
    std::array<real, 1> steps_level {{7}};

    std::array<real,8> steps_level;
    for(size_t i = 0; i < steps_level.size(); ++i) {
         steps_level[i] = 4+i;
    }
    std::array<real,1> steps_epsilon {{0.0001}};
    */

    // setup output stream
    std::ofstream file("/tmp/output.dat");
    ///////////// CONFIG END //////////////////////

    enum {
          yTheory = 0
        , yGridRegular
        , yGridMulti
    };

    file << "# format: level N epsilon norm" << std::endl;

    std::array<std::array<real,2+steps_epsilon.size()>,steps_level.size()> y_values_diff_norm = {{}}; // initialize zero

#ifdef NORM_L_INF
    std::cerr << "use NORM_L_INF (max)" << std::endl;
#else
    std::cerr << "use NORM_L_1 (sum)" << std::endl;
#endif

    for(size_t i_level = 0; i_level < steps_level.size(); ++i_level) {
        const size_t level = steps_level[i_level];
        const size_t N     = pow(1 << level,g_dimension);

        y_values_diff_norm[i_level][yTheory] = g_eps;
        const theory_t theory(level);

        // output row for theory
        // format: level N epsilon norm
        file << boost::format("%d %d %e %e # theory\n")
                % level
                % N
                % steps_epsilon[0]
                % g_eps;

        // regular grid computation
        {

            monores_grid_t grid(level);
            /*
            do {
                grid.timeStep();
            } while(grid.getTime() < simulationTime);
            */
            for (size_t loops = 0; loops < loops_max; ++loops) {
                grid.timeStep();
            }

#ifdef NORM_L_INF
            y_values_diff_norm[i_level][yGridRegular] = g_eps;
#else
            y_values_diff_norm[i_level][yGridRegular] = 0;
#endif
            for (const point_t &point: grid) {
                real diff = std::fabs(theory.at(point.m_index, grid.getTime()) - point.m_phi);
#ifdef NORM_L_INF
                if(y_values_diff_norm[i_level][yGridRegular] < diff) {
                    y_values_diff_norm[i_level][yGridRegular] = diff;
                    // std::cerr << boost::format("x:%03d y:%03d phi: %e\n") % point.m_index[dimX] % point.m_index[dimY] % point.m_phi;
                }
#else
                y_values_diff_norm[i_level][yGridRegular] += diff/N;
#endif
            }
            std::cerr << "finished regular grid with level " << level << std::endl;

            // output row for regular grid
            // format: level N epsilon norm
            file << boost::format("%d %d %e %e # regular\n")
                    % level
                    % N
                    % steps_epsilon[0]
                    % y_values_diff_norm[i_level][yGridRegular];

            // std::cerr << "\n\n\n";
        }

        // multiresolution grid computation (epsilon variable)
        for(size_t i_epsilon = 0; i_epsilon < steps_epsilon.size(); ++i_epsilon) {
            const real epsilon = steps_epsilon[i_epsilon];

            multires_grid_t grid(level, 0, epsilon);
            /*
            do {
                grid.timeStep();
            } while(grid.getTime() < simulationTime);
            */
            for (size_t loops = 0; loops < loops_max; ++loops) {
                grid.timeStep();
            }

            size_t size = grid.size();

            grid.unfold(level);

#ifdef NORM_L_INF
            y_values_diff_norm[i_level][yGridMulti+i_epsilon] = g_eps;
#else
            y_values_diff_norm[i_level][yGridMulti+i_epsilon] = 0;
#endif

            for (const point_t &point: grid) {
                real diff = std::fabs(point.m_phi - theory.at(point.m_index, grid.getTime()));
#ifdef NORM_L_INF
                if(y_values_diff_norm[i_level][yGridMulti+i_epsilon] < diff) {
                    y_values_diff_norm[i_level][yGridMulti+i_epsilon] = diff;
                }
#else
                y_values_diff_norm[i_level][yGridMulti+i_epsilon] += diff/N;
#endif
            }
            std::cerr << "finished level " << level << " eps " << epsilon << " with nodes/N: " << real(size)/N << std::endl;

            // output row for multiresolution grid
            // format: level N epsilon norm
            file << boost::format("%d %d %e %e # multires\n")
                    % level
                    % N
                    % epsilon
                    % y_values_diff_norm[i_level][yGridMulti+i_epsilon];
        }

        file << std::endl << std::endl << std::endl; // empty line to use gnuplot index
    }

    file.close();

    return 0;
}
