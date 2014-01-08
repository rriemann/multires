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
node_base::node_p node_base::increment() const
{
    // do not cross the boundary
    if(this == c_root->boundary(direction)) {
        return nullptr;
    } else if(m_neighbours[direction]) {
        return m_neighbours[direction];
    } else {
        return boundary(direction);
    }
}

node_base::node_p node_base::decrement() const
{
    static const position_t reversed = reverse(direction);

    // do not cross the boundary
    if(this == c_root->boundary(reversed)) {
        return nullptr;
    } else if(m_neighbours[reversed]) {
        return m_neighbours[reversed];
    } else {
        return boundary(reversed);
    }
}

bool node_base::setNodeStateRecursive()
{
    if(m_cached) {
        return m_activeRequirement;
    }

    // Do we? Try to make all existing children inactive
    for(node_u const &child : m_childs) {
        if(child) {
            if(child->setNodeStateRecursive()) {
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
                if(child->setNodeStateRecursive()) {
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
                if(child->setNodeStateRecursive()) {
                    m_activeRequirement = true;
                }
            }
        }

        if(m_activeRequirement) {
            // mark the nearest cousins not deletable as well:
            if(sibling) {
                sibling->setVirtual();
                sibling->setSavetyZone();
            }
            if(cousin_candidate) {
                cousin_candidate->setVirtual();
                cousin_candidate->setSavetyZone();
            }
        }
    }

    if(m_activeRequirement) {

        if(level() < level_t(g_level)) {
            for(size_t i = 0; i < childsByDimension; ++i) {
                createNode(position_t(i)); // create node, if not existing
                m_childs[i]->setSavetyZone();
            }
        }
        m_virtualRequirement = true;
        m_derivative = derivative();
    }

    m_cached = true;

    return m_activeRequirement;
}

void node_base::cleanUpRecursive()
{
    for(node_u &child : m_childs) {
        if(child) {
            child->cleanUpRecursive();
            if(!child->isVirtual() && !child->isSavetyZone()) {
                child.reset();
            }
        }
    }
}

real node_base::derivative() const
{
    // return (neighbour(direction)->property() - neighbour(reverse(direction))->property())/(neighbour(direction)->center(dimX) - neighbour(reverse(direction))->center(dimX));
    // assert(neighbour(reverse(direction))->level() == neighbour(direction)->level());
    assert(g_velocity > 0);
    // for positive velocity we use the look-behind derivative
    return (this->property() - neighbour(reverse(direction))->property())/(this->center(dimX) - neighbour(reverse(direction))->center(dimX));
}

void node_base::timeStepRecursive()
{
    if(m_activeRequirement || m_savetyRequirement) {
        m_property = m_property - g_velocity*g_timestep*m_derivative;
        for(node_u const &child : m_childs) {
            if(child) {
                child->timeStepRecursive();
            }
        }
    } else {
        m_property = interpolation();
        // ^- cut off the datail
    }
    // if m_cached is not set to false, the derivative is not computed again
    m_cached = false;
    m_virtualRequirement = false;
    m_savetyRequirement  = false;
    m_activeRequirement = false;
}

void node_base::timeStep()
{
    assert(this == c_root.get());

    real derivativeEgo = 0;
    node_p nodeEgo;

    if(c_boundaryCondition == bcPeriodic) {
        assert(dimensions == 1);

        // we calculate the PDE for the right boundary manually
        nodeEgo = c_root->boundary(posRight);
        node_p boundaryRight = c_root->boundary(posLeft )->neighbour(posRight);
        // node_p boundaryLeft  = c_root->boundary(posRight)->neighbour(posLeft );

        nodeEgo->setNeighbour(boundaryRight, posRight);
        // c_root->boundary(posLeft)->setNeighbour(boundaryLeft, posLeft);

        derivativeEgo = nodeEgo->derivative();
    }

    updateDerivativeRecursive();
    timeStepRecursive();

    if(c_boundaryCondition == bcPeriodic) {

        nodeEgo->m_property  = nodeEgo->m_property - g_velocity*g_timestep*derivativeEgo;

        // copy the result to the left boundary
        c_root->boundary(posLeft)->m_property = nodeEgo->m_property;
    }

    optimizeTree();
}

void node_base::optimizeTree()
{
    setNodeStateRecursive();

    if(c_boundaryCondition == bcPeriodic) {
        node_p boundaryRight = c_root->boundary(posLeft )->neighbour(posRight);
        node_p boundaryLeft  = c_root->boundary(posRight)->neighbour(posLeft );

        int levelDiff = int(boundaryRight->level()) - int(boundaryLeft->level());
        assert(abs(levelDiff) <= 1);

        if(levelDiff > 0) {
            boundaryLeft->unpackRecursive(level_t(levelDiff));
            boundaryLeft = c_root->boundary(posRight)->neighbour(posLeft);
        } else if(levelDiff < 0) {
            boundaryRight->unpackRecursive(level_t(-levelDiff));
            boundaryRight = c_root->boundary(posLeft)->neighbour(posRight);

        }

        assert(boundaryRight->level() == boundaryLeft->level());

        // make sure, that both outermost (inactive) nodes have the same level
        bool done = false;
        do {
            if(boundaryRight->isVirtual() || boundaryLeft->isVirtual()) {
                boundaryRight->setVirtual();
                boundaryLeft ->setVirtual();
                done = true;
            } else {
                boundaryRight = boundaryRight->parent();
                boundaryLeft  = boundaryLeft ->parent();
            }
        } while(!done);
    }

    cleanUpRecursive();
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
    }
}

node_base::node_p node_base::createRoot(const std::vector<real> &boundary_value, const propertyGenerator_t &propertyGenerator, level_t levels, boundaryCondition_t boundaryCondition)
{
    assert(boundary_value.size() == childsByDimension);

    c_propertyGenerator = propertyGenerator;

    c_boundaryCondition = boundaryCondition;

    node_p_array boundaries = {};
    const node_p_array empty = {};
    real sum = 0;
    for(size_t i = 0; i < boundary_value.size(); ++i) {
        // TODO these destruction of these edge objects is not properly handled as there are no childs of anything
        realarray edgecenter = {{boundary_value[i]}};
        boundaries[i] = node_p(new node_base(edgecenter, position_t(i), lvlBoundary, empty));
        sum += boundary_value[i];
    }
    realarray rootcenter = {{sum/2}};
    c_root = node_u(new node_base(rootcenter, posRoot, lvlRoot, boundaries));


    c_root->unpackRecursive(levels);
    c_root->initPropertyRecursive();

    return c_root.get();
}

/*!
 * \brief node_base::unpack populates this node with new child nodes
 * \param level precises the number of child generations to create relative to the this node
 */
void node_base::unpackRecursive(const level_t level)
{
    if(level > lvlNoChilds) { // there is still a need of children ;)
        for(size_t i = 0; i < childsByDimension; ++i) {
            createNode(position_t(i));
            m_childs[i]->unpackRecursive(level_t(level - 1));
        }
        // m_cached = false;
    }
}

void node_base::updateDerivativeRecursive()
{
    updateDerivative();
    for(node_u &child : m_childs) {
        if(child) {
            child->updateDerivativeRecursive();
        }
    }
}

void node_base::initPropertyRecursive()
{
    for(node_u &child : m_childs) {
        if(child) {
            child->initPropertyRecursive();
        }
        m_property = c_propertyGenerator(m_center);
    }
}

/*!
 * \brief node_base::interpolation return the interpolation value
 *        based on surrounding elements (only elements with lower level are used)
 * \return the actual value for the interpolation
 */
real node_base::interpolation() const
{
#ifndef NDEBUG
    // only in debugging mode interpolation should be could on the boundary
    if(m_level == lvlBoundary) {
        return m_property;
    }
#endif
    assert(m_level > lvlBoundary);
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
    // , m_center initialization (make m_center const) TODO
    , m_property(interpolation())
{
    assert(level > lvlRoot);
    position_t reversed = reverse(position);

    parent->setNeighbour(this, position);
    m_boundaries[position]->setNeighbour(this, reversed);

    assert(boundary(reversed  )->level() + 1 == m_level);
    assert(boundary(m_position)->level() + 2 <= m_level);

    m_center[dimX] = (parent->center(dimX)+parent->boundary(position)->center(dimX))/2;
}

node_base::node_base(realarray center, node_base::position_t position, node_base::level_t level, const node_p_array &boundaries)
    : m_parent(nullptr)
    , m_position(position)
    , m_level(level)
    , m_boundaries(boundaries)
    , m_center(center)
    , m_property(c_propertyGenerator(center))
{
    assert(level < lvlFirst);
}

real node_base::epsilon = EPSILON;
node_base::node_u node_base::c_root = node_u(nullptr);
node_base::boundaryCondition_t node_base::c_boundaryCondition = node_base::bcIndependent;

node_base::propertyGenerator_t node_base::c_propertyGenerator = node_base::propertyGenerator_t();

#include "node_base.hpp"
