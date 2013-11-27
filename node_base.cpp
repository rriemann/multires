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

#include "node_base.hpp"

void node_base::setupChild(const position_t position)
{
    node_p child = factory(shared_from_this(), position, level_t(m_level-1));

    /*
    // inherit the boundary value of this parent to the child node
    child->setNeighbour(this->neighbour(position), position);
    // set the boundary value of this parent value to the childs property
    this->setNeighbour(child, position);
    */

    position_t reversed_position = reverse(position);
    const node_p &boundary = this->neighbour(position);
    // transfer ownership of boundary
    child->setNeighbour(boundary, position);
    // point boundary to closest node (our child)
    boundary->setNeighbour(child, reversed_position);
    // if there is a child, it is always our closest neighbour
    this ->setNeighbour(child, position);
    // e.g. the left child has its parent to the right
    child->setNeighbour(shared_from_this(), reversed_position);

    m_childs[position] = child;
}

void node_base::setupChildren(level_t level)
{
    if(level > m_level) { // there is still a need of children ;)
        assert(dimension == 1); // only support 1D
        // child front
        for(size_t position = int(posLeft); position <= int(posRight); ++position) {
            setupChild(position_t(position));
        }
        /*
        for(size_t i = 0; i < childsByDimension; ++i) {
            if(m_childs[i].get()) {
                m_childs[i]->setupChildren();
            }
        }
        */
    }
}

#include "node_base.hpp"
