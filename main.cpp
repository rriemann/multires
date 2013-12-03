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
    if(x > 0.3 && x < 0.7) {
        return 1.0;
    } else {
        return 0.0;
    }
}

int main()
{
    // generation of childrens, e.g.: only root = 0, grand-children = 2
    int level = 2;
    // total number of nodes, including (childsbyDimension) boundary elements
    uint N     = pow(2, level + 1) - 1 + node_t::childsByDimension;
    real x0    = -1.0;
    real x1    = +1.0;

    real width = x1 - x0;

    real dx    = width / (N-1);

    node_tp no_parent(NULL);
    node_tp left  = node_t::factory(no_parent, node_t::posLeft,  node_t::lvlBoundary);
    node_tp right = node_t::factory(no_parent, node_t::posRight, node_t::lvlBoundary);

    left ->setCenter(x0, node_t::dimX);
    right->setCenter(x1, node_t::dimX);

    node_tp root  = node_t::factory(no_parent, node_t::posRoot, node_t::lvlRoot);
    node_t::setRoot(root);

    root->setNeighbour(left);
    root->setNeighbour(right);
    root->setCenter((x0+x1)/2, node_t::dimX);

    // create children in memory
    root->unpack(node_t::level_t(level));

    int i = 0;
    std::for_each(node_iterator(left), node_iterator(), [&i,x0,dx](node_base &node) {
        node.m_property = f_eval2(x0+i*dx);
        ++i;
    });


    root->isActive();

    std::copy(node_iterator(left), node_iterator(),
              std::ostream_iterator<node_base>(std::cout, "\n"));
    std::cout << std::endl;

    std::ofstream file("/tmp/output.txt");
    std::for_each(node_iterator(left), node_iterator(), [&file](node_base &node) {
         file << boost::format("%e %e %d\n") % node.center() % node.m_property % node.active();
    });
    file.close();
    return 0;
}
