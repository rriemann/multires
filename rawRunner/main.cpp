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

// using namespace std;

#ifndef REGULAR
#include "node/node_iterator.hpp"
#else
#include "regular/regular_base.hpp"
#endif

#include "functions.h"

int main()
{
    // generation of childrens, e.g.: only root = 0, grand-children = 2
    // total number of nodes, including (childsbyDimension) boundary elements

    std::vector<real> boundaries = {x0, x1};

    propertyGenerator_t f_eval = f_eval_gauss;

    real simulationTime = g_span/g_velocity*50; // 50 periods

#ifndef REGULAR
    node_tp root = node_t::createRoot(boundaries, f_eval, node_t::level_t(g_level), bcPeriodic, false);
    root->optimizeTree();
#else
    regular_tp root = regular_t::createRoot(f_eval, g_level, bcPeriodic);
#endif


    size_t count_nodes = (1 << g_level) + 1;

    auto start = std::chrono::steady_clock::now();
    // for(size_t timestep = 0; timestep < 100; ++timestep) {
    do {
        root->timeStep();
    } while(root->getTime() < simulationTime);
    auto done = std::chrono::steady_clock::now();
    std::cerr << "time passed: " << root->getTime() << std::endl;
    double elapsed_time = std::chrono::duration_cast<std::chrono::duration<double>>(done - start).count();
    std::cerr << "calculation time: " << elapsed_time << std::endl;

    size_t count_nodes_packed = 0;

#ifndef REGULAR
    // output file
    std::ofstream file("/tmp/output.txt");
    std::for_each(node_iterator(root->boundary(node_t::posLeft)), node_iterator(), [&](node_base &node) {
        ++count_nodes_packed;

        // std::cerr << node << std::endl;

        file << boost::format("%e %e %e %e %e %e\n")
                % node.center(dimX)
                % node.m_property
                // % ((node.level() > node_t::lvlBoundary) ? node.interpolation() : node.m_property)
                % 12.0
                % ((node.level() > node_t::lvlRoot) ? node.level() : 0)
                % (node.is(node_t::typeActive)     ? ((node.level() > node_t::lvlRoot) ? (pow(2,-node.level())) : 1) : 0)
                % (node.is(node_t::typeSavetyZone) && !node.is(node_t::typeActive) ? ((node.level() > node_t::lvlRoot) ? (pow(2,-node.level())) : 1) : 0);
    });
#else
    // output file
    std::ofstream file("/tmp/output_regular.txt");
    for(size_t i = 0; i < root->size(); i++) {
        file << boost::format("%e %e\n")
                % root->getData().at(i)
                % root->getCenter().at(i);
    }
#endif
    file.close();
    std::cerr << boost::format("pack rate: %d/%d = %f\n") % count_nodes_packed % count_nodes % (real(count_nodes_packed)/count_nodes);
    // std::cerr << "try: gnuplot -p -e \"set style fill solid 1.0; set boxwidth 0.005; plot '/tmp/output.txt' using 1:2 with lines, '' using 1:5 with boxes, '' using 1:6 with boxes\"" << std::endl;
    return 0;
}
