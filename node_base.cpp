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
template <class node_derived>
typename node_base<node_derived>::node_p node_base<node_derived>::increment() const
{
    if(m_neighbours[direction]) {
        return m_neighbours[direction];
    } else {
        return m_boundaries[direction];
    }
}

template <class node_derived>
typename node_base<node_derived>::node_p node_base<node_derived>::decrement() const
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
template <class node_derived>
bool node_base<node_derived>::pack()
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
    return ((!keep) && (fabs(static_cast<node_derived*>(this)->detail()) < epsilon));
}


/*!
 * \brief node_base::setupChild initializes a new child node at the given position
 * \param position
 */
template <class node_derived>
void node_base<node_derived>::createNode(const position_t position)
{
    assert(!m_childs[position]); // full stop if there is already a child
    node_p node = new node_derived(this, position, level_t(m_level+1));
    m_childs[position] = node_u(node);
}

/*!
 * \brief node_base::unpack populates this node with new child nodes
 * \param level precises the number of child generations to create relative to the this node
 */
template <class node_derived>
void node_base<node_derived>::unpack(const level_t level)
{
    if(level > lvlNoChilds) { // there is still a need of children ;)
        for(size_t i = 0; i < childsByDimension; ++i) {
            createNode(position_t(i));
            m_childs[i]->unpack(level_t(level - 1));
        }
    }
}

template <class node_derived>
node_base<node_derived>::~node_base()
{
    if(m_level > lvlRoot) {
        position_t reversed = reverse(m_position);
        node_p boundary = m_boundaries[m_position];
        m_parent->setNeighbour(boundary, m_position);
        boundary->setNeighbour(m_parent, reversed);
    }
}

template <class node_derived>
node_base<node_derived>::node_base(const node_p &parent, node_base::position_t position, node_base::level_t level)
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
    }
}

template <class node_derived>
real node_base<node_derived>::epsilon = EPSILON;

template <class node_derived>
typename node_base<node_derived>::node_u node_base<node_derived>::c_root = node_u(nullptr);

#include "node_base.hpp"
