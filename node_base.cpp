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

#include <cmath>

/*!
 * \brief enables or disables this node
 * \param active state to change this branch for
 */
inline void node_base::setActive(bool active)
{
    /*
    if(m_active == active) {
        return; // don't do anything
    }
    */
    m_active = active;
}

void node_base::setupChild(const position_t position)
{
    node_p child = factory(shared_from_this(), position, level_t(m_level+1));

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

    child->m_property = child->interpolation();

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
        for(size_t i = 0; i < childsByDimension; ++i) {
            if(m_childs[i].get()) {
                m_childs[i]->setupChildren(level);
            }
        }
    }
}

void node_base::multiResolutionTrafo()
{
    // initial question: can we disable this node?
    for(size_t i = int(posLeft); i < childsByDimension; ++i) {
        if(m_childs[i]->active()) {
            // we found one active child node. So this node has to stay as well.
            return;
        }
    }
    m_property = detail();
    if(fabs(m_property) < epsilon) {
        // disable this node
        m_property = 0;
        setActive(false);
    }
}

/*!
 * \brief node_base::interpolation return the interpolation value
 *        based on two surrounding elements
 * \return the actual value for the interpolation
 */
real node_base::interpolation() const
{
    assert(dimension == 1);
    // test for boundary: the parent is never a boundary:
    // check only other direction
    if(m_neighbours[m_position]->level() == lvlBoundary) {
        return (m_neighbours[m_position]->property() + parent()->property())/2;
    } else {
        return (parent()->property()+parent()->parent()->property())/2;
    }
}

real node_base::epsilon = EPSILON;

#include "node_base.hpp"
