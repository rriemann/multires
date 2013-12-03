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

node_p node_base::next() const
{
    position_t reversed = reverse(direction);
    if((m_level == lvlBoundary) && (m_position == reversed)) {
        // we start at the boundary and need to find the closest child
        node_t *close = root().get();
        node_t *closer;
        while((closer = close->child(m_position).get())) {
            close = closer;
        }
        return close->shared_from_this();
    } else if(m_childs[direction].get() == NULL) {
        // go go up in the tree, we can use the neighbour pointers
        return m_neighbours[direction];
    } else {
        // to go down the tree, we have to find the closest child
        node_t *close     = child(direction).get();
        node_t *closer;
        while((closer = close->child(reversed).get())) {
            close = closer;
        }
        return close->shared_from_this();
    }
}

void node_base::setActive(tribool active)
{
    if(m_active == active) {
        return;
    }
    m_active = active;
}

/*!
 * \brief node_base::shortCircuit disables the branch of child nodes if possible
 *
 * \return bool if the branch could be replaced by interpolated values assuming a maximal error of epsilon
 */

bool node_base::isActive()
{
    if(!boost::logic::indeterminate(m_active)) {
        return m_active;
    }
    // test if children can be interpolated
    bool ok = false;
    for(size_t i = 0; i < childsByDimension; ++i) {
        // we use here the lazy evaluation of &&, otherwise: segfault
        if(m_childs[i].get()) {
            if(m_childs[i]->isActive()) {
                ok = true;
            }
        }
    }
    if((fabs(detail()) > epsilon)) {
        setActive(true);
        return true;
    } else {
        setActive(false);
        return false;
    }
}

void node_base::setupChild(const position_t position, const level_t level)
{
    assert(m_childs[position].get() == NULL); // full stop if there is already a child

    node_p child = factory(shared_from_this(), position, level_t(m_level+1));

    assert(dimensions == 1);
    child->unpack(level);

    m_childs[position] = child;
}

/*!
 * \brief node_base::unpack populates this node with new child nodes
 * \param level precises the number of child generations to create relative to the this node
 */
void node_base::unpack(const level_t level)
{
    if(level > lvlNoChilds) { // there is still a need of children ;)
        for(size_t i = 0; i < childsByDimension; ++i) {
            setupChild(position_t(i), level_t(level - 1));
        }
    }
}

/*!
 * \brief node_base::interpolation return the interpolation value
 *        based on two surrounding elements
 * \return the actual value for the interpolation
 */
real node_base::interpolation() const
{
#ifndef NO_DEBUG
    if(m_level == lvlBoundary) {
        return m_property;
    }
#endif
    real property = 0;
    // # TODO explicitly unroll this loop?
    for(size_t i = 0; i < childsByDimension; ++i) {
        property += m_neighbours[i]->property();
    }
    return property/childsByDimension;
}

node_base::node_base(const node_p &parent, node_base::position_t position, node_base::level_t level)
    : m_parent(parent)
    , m_position(position)
    , m_level(level)
    , m_active(boost::logic::indeterminate)
{
    if(level > lvlRoot) {
        m_neighbours[reverse(position)] = parent;
        m_neighbours[position] = parent->neighbour(position);

        m_center[dimX] = (parent->center()+parent->neighbour(position)->center())/2;
        m_property = interpolation();
    }
}

/*!
 * \brief removes the child at position from this node
 * \param position of the child to be removed
 *
 * \note   this does not necessarily delete the child. It will be deleted
 *         on removal of the last pointer
 *
 * \sa std::shared_ptr
 */
void node_base::detachChild(const node_base::position_t position)
{
    assert(m_childs[position].get()); // there must be a child

    m_childs[position]->detach();
    m_childs[position].reset();
}

/*!
 * \brief node_base::detach will prepare this element for removal
 * \return the node_base to replace this node as a neighbour
 */
void node_base::detach()
{
    for(size_t i = 0; i < childsByDimension; ++i) {
        m_childs[i]->detachChild(position_t(i));
        m_neighbours[i].reset();
    }
    m_parent.reset();
}

real node_base::epsilon = EPSILON;
node_p node_base::c_root = node_p();

#include "node_base.hpp"
