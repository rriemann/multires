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
#include <iostream>
#include <algorithm>
#include <functional>

// using namespace std;

#include "node_iterator.hpp"

int main()
{
    /*
    // RealVector data = {0, 1, 2, 3, 4, 5, 6, 7, 8};
    Node_ptr no_parent(NULL);
    Node_ptr front = Node::factory(no_parent, Node::posTopRightFront);
    front->setProperty(0);
    Node_ptr back  = Node::factory(no_parent, Node::posTopRightBack);
    back ->setProperty(1);
    Node_ptr root  = Node::factory(no_parent, Node::posRoot, 4);

    root->setupChildren();

    NodeIterator it(front);
    for(; it < back.get(); ++it) {
        std::cout << (*it) << std::endl;
    }
    */

    node_tp no_parent(NULL);
    node_tp left  = node_t::factory(no_parent, node_t::posLeft);
    node_tp right = node_t::factory(no_parent, node_t::posRight);

    node_tp root  = node_t::factory(no_parent, node_t::posRoot);
    root->setNeighbour(left);
    root->setNeighbour(right);

    std::copy(
        node_iterator(left), node_iterator()
      , std::ostream_iterator<node_base>(std::cout, " : ")
    );
    std::cout << std::endl;

    return 0;
}
