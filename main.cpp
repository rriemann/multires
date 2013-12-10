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

#include "node_iterator.hpp"

real f_eval(real x) {
    if(x < -0.5) {
        return 0;
    } else if (x < 0) {
        return 1;
    } else if (x < 0.5) {
        return 4*(x-0.5)*(x-0.5);
    } else {
        return 2*(x-0.5);
    }
}

real f_eval2(real x) {
    if(x > 0.25 && x < 0.75) {
        return 1.0;
    } else {
        return 0.0;
    }
}

int main()
{
    // generation of childrens, e.g.: only root = 0, grand-children = 2
    int level = 16;
    // total number of nodes, including (childsbyDimension) boundary elements
    real x0    = -1.0;
    real x1    = +1.0;

    std::vector<real> boundary = {x0, x1};
    node_tp root  = node_t::createRoot(boundary);

    // create children in memory
    root->unpack(node_t::level_t(level));

    std::for_each(node_iterator(root->neighbour(node_t::posLeft).get()), node_iterator(), [](node_base &node) {
        node.m_property = f_eval(node.center());
    });

    root->pack();

    // output command line
    std::for_each(node_iterator(root->neighbour(node_t::posLeft).get()), node_iterator(), [](node_base &node) {
        std::cout << node << std::endl;
    });

    // output file
    std::ofstream file("/tmp/output.txt");
    std::for_each(node_iterator(root->neighbour(node_t::posLeft).get()), node_iterator(), [&file](node_base &node) {
        file << boost::format("%e %e %e\n") % node.center() % node.m_property % node.interpolation();
    });
    file.close();
    return 0;
}
