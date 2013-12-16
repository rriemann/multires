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
        return boundary(direction);
    }
}

node_p node_base::decrement() const
{
    static const position_t reversed = reverse(direction);

    if(m_neighbours[reversed]) {
        return m_neighbours[reversed];
    } else {
        return boundary(reversed);
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
    bool keep = false; // keep means: do not pack
    for(size_t i = 0; i < childsByDimension; ++i) {
        node_u &child     = m_childs[i];
        if(child) {
            if(child->pack()) {
                child.reset();
            } else {
                keep = true;
            }
        }
    }
    return ((!keep) && (fabs(detail()) < epsilon));
}

/*!
 * \brief node_base::pack2
 * \return bool if this node can be deleted.
 */
bool node_base::pack2()
{
    /*
     * important notice:
     * never ever call pack2 recursive on a node with same or lower level than
     * the level of the current node. This would create an infinite loop
     */
    bool keep = false; // keep means: do not pack

    // do I have children?
    for(size_t i = 0; i < childsByDimension; ++i) {
        node_u &child = m_childs[i];
        if(child) {
            if(child->pack2()) {
                child.reset();
            } else {
                keep = true;
            }
        }
    }

    // two reasons to not pack:
    // - I have children who cannot be deleted.
    // - My value is too important.
    if(keep || (fabs(detail()) > epsilon)) {
        return false;
    }

    // let's see if we have to chance to return true?

    position_t reversed = reverse(position());

    // check nephew
    node_u &silbling = parent()->child(reversed);
    if(silbling) {
        node_u &child = silbling->child(position());
        if(child) {
            if(child->pack2()) {
                child.reset();
            } else {
                return false;
            }
        }
    }

    // check grand-nephew
    node_p bound = boundary(position());
    node_p candidate = bound->neighbour(position());
    // we only do something when there is a grand-nephew:
    // - maybe there is only a grand-cousin
    // - maybe the grand-nephew has even children
    if(candidate && candidate->level() > m_level) {
        // ok, there must be a grand-nephew: we found him already,
        // or at least his children
        // we iterate until we found the parent of the grand-nephew,
        // which should be our cousin (same level)
        do {
            candidate = candidate->parent();
        } while(candidate->level() > m_level);

        assert(m_level == candidate->level());

        // doesn't make much sense to get first the parent and then the child, heh?
        // consider: we need to use the unique_ptr to delete the child, and
        //           the unique_ptr can only be accessed by the owner of the pointer,
        //           which is the parent node.
        node_u &child = candidate->child(reversed);
        if(child) {
            if(child->pack2()) {
                child.reset();
            } else {
                return false;
            }
        }
    }

    return true;
}

/*!
 * \brief node_base::pack3 prepares the compression of the tree
 * \return bool false if there is at least one non-virtual node in this branch (maybe this)
 */
bool node_base::isActive()
{
    if(m_cached) {
        return m_activeRequirement;
    }

    // Do we? Try to make all existing children inactive
    for(size_t i = 0; i < childsByDimension; ++i) {
        node_u &child = m_childs[i];
        if(child) {
            if(child->isActive()) {
                m_activeRequirement = true;
            }
        }
    }

    /*
     * important notice:
     * never ever call isActive recursive on a node with same or lower level than
     * the level of the current node. This would create an infinite loop
     */

    // My value is too important.
    if(fabs(detail()) > epsilon) {
        m_activeRequirement = true;
    }

    if(level() > lvlRoot) {
        position_t reversed = reverse(position());

        // check nephew
        node_u &sibling = parent()->child(reversed);
        if(sibling) {
            node_u &child = sibling->child(position());
            if(child) {
                if(child->isActive()) {
                    m_activeRequirement = true;
                }
            }
        }

        // check grand-nephew
        node_p cousin_candidate = boundary(position())->neighbour(position());
        // we only do something when there is a grand-nephew:
        // - maybe there is only a grand-cousin
        // - maybe the grand-nephew has even children
        if(cousin_candidate && cousin_candidate->level() > m_level) {
            // ok, there must be a grand-nephew: we found him already,
            // or at least his children
            // we iterate until we found the parent of the grand-nephew,
            // which should be our cousin (same level)
            do {
                cousin_candidate = cousin_candidate->parent();
            } while(cousin_candidate->level() > m_level);

            assert(m_level == cousin_candidate->level());

            // doesn't make much sense to get first the parent and then the child, heh?
            // consider: we need to use the unique_ptr to delete the child, and
            //           the unique_ptr can only be accessed by the owner of the pointer,
            //           which is the parent node.
            node_u &child = cousin_candidate->child(reversed);
            if(child) {
                if(child->isActive()) {
                    m_activeRequirement = true;
                }
            }
        }

        if(m_activeRequirement) {
            // mark the nearest cousins not deletable as well:
            if(sibling) {
                sibling->setVirtual();
            }
            if(cousin_candidate) {
                cousin_candidate->setVirtual();
            }
        }
    }

    if(m_activeRequirement) {
        m_virtualRequirement = true;
        updateDerivative();
    }

    m_cached = true;

    return m_activeRequirement;
}

void node_base::cleanUp()
{
    for(size_t i = 0; i < childsByDimension; ++i) {
        node_u &child = m_childs[i];
        if(child) {
            child->cleanUp();
            if(!child->isVirtual()) {
                child.reset();
            }
        }
    }
}

void node_base::flow()
{
    if(m_activeRequirement) {
        m_property = m_property - velocity*timestep*derivative();
        for(size_t i = 0; i < childsByDimension; ++i) {
            node_u &child = m_childs[i];
            if(child) {
                child->flow();
            }
        }
        m_cached = false;
        m_virtualRequirement = false;
        m_activeRequirement = false;
    } else {
        m_property = interpolation();
    }
}

/*!
 * \brief node_base::setupChild initializes a new child node at the given position
 * \param position
 */
void node_base::createNode(const position_t position)
{
    if(!m_childs[position]) {
        assert(dimensions == 1);
        node_p_array boundaries;
        boundaries[reverse(position)] = this;
        boundaries[position] = this->boundary(position);
        node_p node = new node_base(this, position, level_t(m_level+1), boundaries);
        m_childs[position] = node_u(node);
        m_cached = false;
    }
}

node_p node_base::createRoot(const std::vector<real> &boundary_value)
{
    assert(boundary_value.size() == childsByDimension);

    node_p_array boundaries;
    const node_p_array empty = {};
    real sum = 0;
    for(size_t i = 0; i < boundary_value.size(); ++i) {
        // TODO these destruction of these edge objects is not properly handled as there are no childs of anything
        node_p edge = node_p(new node_base(position_t(i), lvlBoundary, empty));
        sum += boundary_value[i];
        edge->setCenter(boundary_value[i]);
        boundaries[i] = edge;
    }
    c_root = node_u(new node_base(posRoot, lvlRoot, boundaries));
    c_root->setCenter(sum/2);
    c_root->m_property = c_root->interpolation();
    return c_root.get();
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
        // m_activeChilds = boost::logic::indeterminate;
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
        property += boundary(position_t(i))->property();
    }
    return property/childsByDimension;
}

node_base::~node_base()
{
    if(m_level > lvlRoot) {
        position_t reversed = reverse(m_position);
        node_p bound = boundary(m_position);
        m_parent->setNeighbour(bound, m_position);
        bound->setNeighbour(m_parent, reversed);
    }
}

node_base::node_base(const node_p &parent, node_base::position_t position, node_base::level_t level, const node_p_array &boundaries)
    : m_parent(parent)
    , m_position(position)
    , m_level(level)
    , m_boundaries(boundaries)
    , m_neighbours(boundaries)
    // , m_active(boost::logic::indeterminate)
{
    assert(level > lvlRoot);
    position_t reversed = reverse(position);

    parent->setNeighbour(this, position);
    m_boundaries[position]->setNeighbour(this, reversed);

    assert(boundary(reversed  )->level() + 1 == m_level);
    assert(boundary(m_position)->level() + 2 <= m_level);

    m_center[dimX] = (parent->center()+parent->boundary(position)->center())/2;
    m_property = interpolation();
}

node_base::node_base(node_base::position_t position, node_base::level_t level, const node_p_array &boundaries)
    : m_parent(nullptr)
    , m_position(position)
    , m_level(level)
    , m_boundaries(boundaries)
{
    assert(level < lvlFirst);
}

real node_base::epsilon = EPSILON;
node_u node_base::c_root = node_u(nullptr);

#include "node_base.hpp"
