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

#include "functions.h"


#include <limits>
static const real eps = std::numeric_limits<real>::epsilon();

int main()
{
    // generation of childrens, e.g.: only root = 0, grand-children = 2
    // total number of nodes, including (childsbyDimension) boundary elements

    size_t level = 6;
    size_t N = (2 << level) + 1;
    real simulationTime = g_span/g_velocity;
    const size_t series = 10;

    enum {
          yTheory = 0
        , yRegularGrid
        , yMultiGrid
    };

    std::array<real,2+series> y_values_l_inf = {}; // initialize zero
    y_values_l_inf[yTheory] = eps;
    std::vector<real> y_values_theory;
    std::vector<real> epsilon_values;
    epsilon_values.push_back(N*eps); // theory
    epsilon_values.push_back(N*eps*10); // regular grid

    // tree grid and analytic computation
    for(size_t i = 0; i < series; ++i) {
        std::vector<real> boundaries = {x0, x1};
        node_tp root = node_t::createRoot(boundaries, f_eval_triangle, node_t::level_t(level), bcPeriodic, false);
        const real epsilon = 0.0001*pow(2,i);
        root->setEpsilon(epsilon);
        root->optimizeTree();

        while(root->getTime() < simulationTime) {
            root->timeStep();
        }
        size_t elements = std::distance(node_iterator(root->boundary(node_t::posLeft)), node_iterator());

        root->unpackRecursiveTesting(node_t::level_t(level));

        size_t checkN = 0;
        // std::for_each(node_iterator(root->boundary(node_t::posLeft)), node_iterator(), [&,series](node_base &node) {
        for(node_iterator iter = node_iterator(root->boundary(node_t::posLeft)); iter != node_iterator(); iter++) {
            node_t &node = *iter;
            // l_inf is given by the maximum
            if(i == 0) {
                y_values_theory.push_back(node.propertyTheory());
                /*
                if(y_values_l_inf[yTheory] < node.propertyTheory()) {
                    y_values_l_inf[yTheory] = node.propertyTheory();
                }
                */
            }
            real diff = std::fabs(node.property() - node.propertyTheory());
            /*
            if(y_values_l_inf[yMultiGrid+i] < diff) {
                y_values_l_inf[yMultiGrid+i] = diff;
            }
            */
            y_values_l_inf[yMultiGrid+i] += diff;
            checkN++;
        }
        // });
        assert(N == checkN);

        epsilon_values.push_back(epsilon);
        std::cerr << "finished series " << i << " (" << epsilon << ") with nodes/N: " << real(elements)/N << std::endl;
    }

    // regular grid computation
    {
        regular_tp root = regular_t::createRoot(f_eval_triangle, level, bcPeriodic);
        assert(root->size() == N);
        while(root->getTime() < simulationTime) {
            root->timeStep();
        }

        for(size_t i = 0; i < root->size(); i++) {
            // l_inf is given by the maximum
            real diff = std::fabs(y_values_theory.at(i) - root->getData().at(i));
            /*
            if(y_values_l_inf[yRegularGrid] < diff) {
                y_values_l_inf[yRegularGrid] = diff;
            }
            */
            y_values_l_inf[yRegularGrid] += diff;
        }
        std::cerr << "finished regular grid" << std::endl;
    }

    // output
    std::ofstream file("/tmp/output.txt");
    for(size_t i = 0; i < series+2; i++) {
        file << boost::format("%e %e\n") % epsilon_values.at(i) % y_values_l_inf.at(i);
    }
    file.close();

    return 0;
}
