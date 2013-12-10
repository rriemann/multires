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

/**
 * @brief node_base::next is used by the node_base iterator class to get the next node
 * @return node_p shared_ptr to the next node
 *
 * The implementation of the tree doesn't allow to have a very efficent algorith to get the next node.
 * So this method should only be used in the rare cases of output generation
 */
node_p node_base::increment() const
{
    if(m_neighbours[direction]) {
        return m_neighbours[direction];
    } else {
        return m_boundaries[direction];
    }
}

node_p node_base::decrement() const
{
    static const position_t reversed = reverse(direction);

    if(m_neighbours[reversed]) {
        return m_neighbours[reversed];
    } else {
        return m_boundaries[reversed];
    }
}


/*!
 * \brief node_base::pack
 *
 * \return bool if the branch could be replaced by interpolated values assuming a maximal error of epsilon
 */
bool node_base::pack()
{
    /*
    if(!boost::logic::indeterminate(m_active)) {
        return m_active;
    }
    */
    // test if children can be interpolated
    bool keep = false; // keep = do not pack
    for(size_t i = 0; i < childsByDimension; ++i) {
        if(m_childs[i].get()) {
            if(m_childs[i]->pack()) {
                m_childs[i].reset();
            } else {
                keep = true;
            }
        }
    }
    return ((!keep) && (fabs(detail()) < epsilon));
}


/*!
 * \brief node_base::setupChild initializes a new child node at the given position
 * \param position
 */
void node_base::createNode(const position_t position)
{
    assert(!m_childs[position]); // full stop if there is already a child
    node_p node = new node_base(this, position, level_t(m_level+1));
    m_childs[position] = node_u(node);
}

/*!
 * \brief node_base::unpack populates this node with new child nodes
 * \param level precises the number of child generations to create relative to the this node
 */
void node_base::unpack(const level_t level)
{
    if(level > lvlNoChilds) { // there is still a need of children ;)
        for(size_t i = 0; i < childsByDimension; ++i) {
            createNode(position_t(i));
            m_childs[i]->unpack(level_t(level - 1));
        }
    }
}

/*!
 * \brief node_base::interpolation return the interpolation value
 *        based on surrounding elements (only elements with lower level are used)
 * \return the actual value for the interpolation
 */
real node_base::interpolation() const
{
#ifndef NO_DEBUG
    // only in debugging mode interpolation should be could on the boundary
    if(m_level == lvlBoundary) {
        return m_property;
    }
#endif
    real property = 0;
    // # TODO explicitly unroll this loop?
    for(size_t i = 0; i < childsByDimension; ++i) {
        property += m_boundaries[i]->property();
    }
    return property/childsByDimension;
}

node_base::~node_base()
{
    if(m_level > lvlRoot) {
        position_t reversed = reverse(m_position);
        node_p boundary = m_boundaries[m_position];
        m_parent->setNeighbour(boundary, m_position);
        boundary->setNeighbour(m_parent, reversed);
    }
}

node_base::node_base(const node_p &parent, node_base::position_t position, node_base::level_t level)
    : m_parent(parent)
    , m_position(position)
    , m_level(level)
    // , m_active(boost::logic::indeterminate)
{
    if(level > lvlRoot) { // this also filters lvlBoundary
        position_t reversed = reverse(position);
        m_boundaries[reversed] = parent;
        m_boundaries[position] = parent->boundary(position);

        parent->setNeighbour(this, position);
        m_boundaries[position]->setNeighbour(this, reversed);

        m_center[dimX] = (parent->center()+parent->boundary(position)->center())/2;
        m_property = interpolation();
    }
}

real node_base::epsilon = EPSILON;
node_u node_base::c_root = node_u(nullptr);

#include "node_base.hpp"
