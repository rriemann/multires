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

#include "functions.h"

int main()
{
    // generation of childrens, e.g.: only root = 0, grand-children = 2
    // total number of nodes, including (childsbyDimension) boundary elements

    std::vector<real> boundaries = {x0, x1};
    node_tp root = node_t::createRoot(boundaries);

    // create children in memory
    root->unpack(node_t::level_t(level));

    size_t count_nodes = 0;
    std::for_each(node_iterator(root->boundary(node_t::posLeft)), node_iterator(), [&](node_base &node) {
        ++count_nodes;
        node.m_property = f_eval5(node.center());
    });

    for(size_t timestep = 0; timestep < 1; ++timestep) {
        root->isActive();
        root->cleanUp();
        root->flow();
    }

    size_t count_nodes_packed = 0;

    // output file
    std::ofstream file("/tmp/output.txt");
    std::for_each(node_iterator(root->boundary(node_t::posLeft)), node_iterator(), [&](node_base &node) {
        ++count_nodes_packed;

        std::cout << node << std::endl;

        file << boost::format("%e %e %e %e %e %e\n")
                % node.center()
                % node.m_property
                % node.interpolation()
                % ((node.level() > node_t::lvlRoot) ? node.level() : 0)
                % ( node.active() ? ((node.level() > node_t::lvlRoot) ? (pow(2,-node.level())) : 1) : 0)
                % (!node.active() ? ((node.level() > node_t::lvlRoot) ? (pow(2,-node.level())) : 1) : 0);
    });
    file.close();
    std::cerr << boost::format("pack rate: %d/%d = %f\n") % count_nodes_packed % count_nodes % (real(count_nodes_packed)/count_nodes);
    std::cerr << "try: gnuplot -p -e \"set style fill solid 1.0; set boxwidth 0.005; plot '/tmp/output.txt' using 1:2 with lines, '' using 1:5 with boxes, '' using 1:6 with boxes\"" << std::endl;
    return 0;
}
