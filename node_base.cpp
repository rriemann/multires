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
 * \brief node_base::shortCircuit disables the branch of child nodes if possible
 *
 * \return bool if the branch could be replaced by interpolated values assuming a maximal error of epsilon
 */
bool node_base::interpolatable()
{
    // test if children can be interpolated
    for(size_t i = 0; i < childsByDimension; ++i) {
        // we use here the lazy evaluation of ||, otherwise: segfault
        if((!m_childs[i].get()) || (!m_childs[i]->shortCircuit())) {
            return false;
        }
    }
    if(fabs(detail()) > epsilon) {
        return false;
    }
}

void node_base::shortCircuit()
{
    if (interpolatable()) {
        assert(dimension == 1); // neighbour handling is more complicated in dim>1

        // this nodes gets again the neighbor of his parent
        parent()->setNeighbour(shared_from_this());

        setNeighbour(deepNeighbour(m_position));
    }
}

void node_base::setupChild(const position_t position, const level_t level)
{
    assert(m_childs[position].get() == NULL); // full stop if there is already a child

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
    assert(dimension == 1);
    // test for boundary: the parent is never a boundary:
    // check only other direction
    if(m_neighbours[m_position]->level() == lvlBoundary) {
        return (m_neighbours[m_position]->property() + parent()->property())/2;
    } else {
        return (parent()->property()+parent()->parent()->property())/2;
    }
}

/*!
 * \brief node_base::deepNeighbour gives the closest neighbour which is not part of this branch
 * \param position direction to look for the neighbour
 * \return node_p of the neighbour
 */
const node_p node_base::deepNeighbour(const node_base::position_t position) const
{
    if(m_childs[position].get()) { // if there is a child ...
        return m_childs[position]->deepNeighbour(position);
    } else {
        return m_neighbours[position];
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

    this->setNeighbour(deepNeighbour(position));
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
        m_childs[i]->detach();
        m_childs[i].reset();
        m_neighbours[i].reset();
    }
    m_parent.reset();
}

/*!
 * \brief node_base::pack reduces the branch recursively below this node if possible
 */
void node_base::pack()
{
    assert(dimension == 1); // neighbour handling is more complicated in dim>1

    const node_p neighbour = deepNeighbour(m_position)->shared_from_this();

    // reset recursively childs and neighbours to NULL pointer
    detach();

    // this nodes gets again the neighbor of his parent
    parent()->setNeighbour(shared_from_this());
    setNeighbour(neighbour);
}

real node_base::epsilon = EPSILON;

#include "node_base.hpp"
