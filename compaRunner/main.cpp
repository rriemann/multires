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

#include "node/node_iterator.hpp"
#include "regular/regular_base.hpp"
#include "theory_base.hpp"

#include "functions.h"


#include <limits>
static const real eps = std::numeric_limits<real>::epsilon();

realarray center(real x) {
    return realarray{{x}};
}

int main()
{
    ///////////// CONFIG //////////////////////
#define NORM_L_INF // uncomment to use L_1 norm


    real simulationTime = g_span/g_velocity; // 1 period

    /*
    std::array<real, 3> steps_level {{6, 7, 8}};

    std::array<real,10> steps_epsilon;
    for(size_t i = 0; i < steps_epsilon.size(); ++i) {
         steps_epsilon[i] = 0.0001*pow(2,i);
    }
    */

    std::array<real,6> steps_level;
    for(size_t i = 0; i < steps_level.size(); ++i) {
         steps_level[i] = 6+i;
    }
    std::array<real,1> steps_epsilon {{0.0001}};


    propertyGenerator_t f_eval = f_eval_triangle;

    // setup output stream
    std::ofstream file("/tmp/output.txt");
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
        const size_t N     = level2N(level);

        y_values_diff_norm[i_level][yTheory] = eps;
        const theory_base theory(f_eval, level);

        // output row for theory
        // format: level N epsilon norm
        file << boost::format("%d %d %e %e # theory\n")
                % level
                % N
                % eps
                % eps;

        // regular grid computation
        {

            regular_tp root = regular_t::createRoot(f_eval, level, bcPeriodic);
            while(root->getTime() < simulationTime) {
                root->timeStep();
            }
#ifdef NORM_L_INF
            y_values_diff_norm[i_level][yGridRegular] = eps;
#else
            y_values_diff_norm[i_level][yGridRegular] = 0;
#endif
            for(size_t i = 0; i < root->size(); i++) {
                // std::cerr << "theory: " << theory.at(i, root->getTime()) << " exp: " << root->getData().at(i) << std::endl;
                real diff = std::fabs(theory.at(i, root->getTime()) - root->getData().at(i));
#ifdef NORM_L_INF
                if(y_values_diff_norm[i_level][yGridRegular] < diff) {
                    y_values_diff_norm[i_level][yGridRegular] = diff;
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
                    % (eps*10)
                    % y_values_diff_norm[i_level][yGridRegular];
        }

        // multiresolution grid computation (epsilon variable)
        for(size_t i_epsilon = 0; i_epsilon < steps_epsilon.size(); ++i_epsilon) {
            const real epsilon = steps_epsilon[i_epsilon];


            std::vector<real> boundaries = {x0, x1};
            node_tp root = node_t::createRoot(boundaries, f_eval, node_t::level_t(level), bcPeriodic, false);
            root->setEpsilon(epsilon);
            root->optimizeTree();

            while(root->getTime() < simulationTime) {
                root->timeStep();
            }
            size_t elements = std::distance(node_iterator(root->boundary(node_t::posLeft)), node_iterator());

            root->unpackRecursiveTesting(node_t::level_t(level));

#ifdef NORM_L_INF
            y_values_diff_norm[i_level][yGridMulti+i_epsilon] = eps;
#else
            y_values_diff_norm[i_level][yGridMulti+i_epsilon] = 0;
#endif
            std::for_each(node_iterator(root->boundary(node_t::posLeft)), node_iterator(), [&](node_base &node) {
                real diff = std::fabs(node.property() - node.propertyTheory());
#ifdef NORM_L_INF
                if(y_values_diff_norm[i_level][yGridMulti+i_epsilon] < diff) {
                    y_values_diff_norm[i_level][yGridMulti+i_epsilon] = diff;
                }
#else
                y_values_diff_norm[i_level][yGridMulti+i_epsilon] += diff/N;
#endif
            });
            std::cerr << "finished level " << level << " eps " << epsilon << " with nodes/N: " << real(elements)/N << std::endl;

            // output row for multiresolution grid
            // format: level N epsilon norm
            file << boost::format("%d %d %e %e\n")
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
