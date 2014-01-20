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
#include <iostream>

#include <limits>
static const real eps = std::numeric_limits<real>::epsilon();

/**
 * @brief node_base::next is used by the node_base iterator class to get the next node
 * @return node_p shared_ptr to the next node
 *
 * The implementation of the tree doesn't allow to have a very efficent algorith to get the next node.
 * So this method should only be used in the rare cases of output generation
 */
node_base::node_p node_base::forward(const node_base::position_t position) const
{
    // do not cross the boundary
    if(this == c_root->boundary(position)) {
        return nullptr;
    } else {
        return neighbour(position);
    }
}

bool node_base::isActiveTypeRecursive()
{
    // here we determine the type of the node:
    // active or virtual or savetyzone or none

    if(!is(typeCached)) {
        // We try to find active children
        for(node_u const &child : m_childs) {
            if(child) {
                if(child->isActiveTypeRecursive()) {
                    // with active children, we have to be active as well
                    set(typeActive);
                }
            }
        }

        /*
         * important notice:
         * never ever call isActive recursive on a node with same or lower level than
         * the level of the current node. This would create an infinite loop
        */

        if(fabs(detail()) > c_epsilon) {
            // My value is too important. I have to be active to keep it.
            set(typeActive);
        }


        // Now we gonna check our big family. If we are uncle, which means,
        // that we have nephews (child of sibling or of cousin), we have to
        // stay active.
        if(level() > lvlRoot) {
            position_t reversed = reverse(position());

            // check nephew
            node_u &sibling = parent()->child(reversed);
            if(sibling) {
                node_u &child = sibling->child(position());
                if(child) {
                    if(child->isActiveTypeRecursive()) {
                        set(typeActive);
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
                    if(child->isActiveTypeRecursive()) {
                        set(typeActive);
                    }
                }
            }

            if(is(typeSavetyZone)) {
                // mark the nearest cousins not deletable as well:
                if(sibling) {
                    sibling->set(typeVirtual);
                    //sibling->set(typeSavetyZone);
                }
                if(cousin_candidate) {
                    cousin_candidate->set(typeVirtual);
                    //cousin_candidate->set(typeSavetyZone);
                }
            }

            if(is(typeActive)) {
                // mark the nearest cousins not deletable as well:
                if(sibling) {
                    // sibling->set(typeVirtual);
                    sibling->set(typeSavetyZone);
                }
                if(cousin_candidate) {
                    // cousin_candidate->set(typeVirtual);
                    cousin_candidate->set(typeSavetyZone);
                }
            }
        }

        if(is(typeActive)) {
            if(level() < level_t(g_level)) {
                for(size_t i = 0; i < childsByDimension; ++i) {
                    createNode(position_t(i)); // create node, if not existing
                    m_childs[i]->set(typeSavetyZone);
                }
            }
        }
        set(typeCached);
    }

    return is(typeActive);
}

void node_base::cleanUpRecursive()
{
    for(node_u &child : m_childs) {
        if(child) {
            child->cleanUpRecursive();
            if(!child->isVirtual()) {
                child.reset();
            }
        }
    }
}

void node_base::timeStepRecursive()
{
    if(isSavetyZone()) {
        m_property = timeStepValue();
        for(node_u const &child : m_childs) {
            if(child) {
                child->timeStepRecursive();
            }
        }
    } else {
        m_property = interpolation();
        // ^- cut off the datail
    }

    // if type is not unset, the derivative is not computed again
    m_type = typeUnset;
}

real node_base::timeStepValue()
{
    node_p neighbourLeft  = neighbour(posLeft);
    node_p neighbourRight = neighbour(posRight);
    // BEGIN bad hack
    //
    // due to the introduction of the savety-zone, the distance to the
    // neighbours is not equal anymore. To get a symmetric case, we have to skip
    // potentially one neighbour.
    if(neighbourLeft->level() > lvlBoundary && neighbourRight->level() > lvlBoundary && (child(posLeft).get() || child(posRight).get())) {
        int diffLevel = neighbourRight->level() - neighbourLeft->level();
        if(diffLevel >= 1) {
            neighbourRight = neighbourRight->neighbour(posRight);
        } else if(diffLevel <= -1) {
            neighbourLeft  = neighbourLeft->neighbour(posLeft);
        } else {
            // assert(diffLevel == 0);
        }
    }
    // END bad hack

    // Lax-Wendroff method
    // see: http://www.exp.univie.ac.at/cp1/cp1-6/node72.html
    real dx    = (neighbourRight->center(dimX)-neighbourLeft->center(dimX))/2;
    if(level() == lvlBoundary) {
        // we have crossed the border and have to apply the offset
       dx += g_span/2;
    }
    const real alpha = g_velocity*g_timestep/dx;

#ifndef NDEBUG
    assert(dx > 0);

    // check if the neighbours have equal distance
    const real dxr    = neighbourRight->center(dimX)-center(dimX);
    const real dxl    = center(dimX)-neighbourLeft->center(dimX);

    if(level() > lvlBoundary) {
        if(c_boundaryCondition != bcNone) {
            assert(fabs(dxr-dxl) <= eps);
        }
        // check if the neighbours are not too far away
        assert(2*dx < g_span/(1 << level())+eps);
    } else {
        assert(fabs(dxr-dxl+g_span) <= eps);
    }
#endif
    const real er = neighbourRight->m_propertyBackup; // element right (j+1)
    const real el = neighbourLeft ->m_propertyBackup; // element left  (j-1)
    const real property = m_propertyBackup - alpha/2*(er-el-alpha*(er-2*m_propertyBackup+el));

    // assert(fabs(property - m_propertyBackup) < 0.5);

    return property;
}

void node_base::timeStep()
{
    assert(this == c_root.get());
    c_time += g_timestep;

    // we calculate the PDE for the right boundary manually


    node_p nodeEgo = c_root->boundary(posRight);

    // the timeStepRecursive method calls timeStepValue which uses the back!
    // Thus it needs to be uptodate!
    updateBackupValueRecursive();
    for(const node_p &bounding: m_boundaries) {
        bounding->updateBackupValue();
    }

    if(c_boundaryCondition == bcPeriodic) {
        c_root->boundary(posLeft )->setNeighbour(c_root->boundary(posRight)->neighbour(posLeft ), posLeft );
        c_root->boundary(posRight)->setNeighbour(c_root->boundary(posLeft )->neighbour(posRight), posRight);
    }
    timeStepRecursive();

    if(c_boundaryCondition == bcIndependent) {
        // up-wind derivative
        real derivativeEgo = (nodeEgo->propertyBackup() - nodeEgo->neighbour(c_reversed)->propertyBackup())/(nodeEgo->center(dimX) - nodeEgo->neighbour(c_reversed)->center(dimX));
        nodeEgo->m_property = nodeEgo->m_property - g_velocity*g_timestep*derivativeEgo;
    } else if(c_boundaryCondition == bcPeriodic) {
        nodeEgo->m_property = nodeEgo->timeStepValue();

        // copy the result to the left boundary
        c_root->boundary(posLeft)->m_property = nodeEgo->m_property;
    }

    /*
    updateBackupValueRecursive();
    for(const node_p &bounding: m_boundaries) {
        bounding->updateBackupValue();
    }
    */

    optimizeTree();
}

void node_base::optimizeTree()
{
    if(c_boundaryCondition == bcPeriodic) {
        c_root->boundary(posLeft )->setNeighbour(c_root->boundary(posRight)->neighbour(posLeft ), posLeft );
        c_root->boundary(posRight)->setNeighbour(c_root->boundary(posLeft )->neighbour(posRight), posRight);
    }

    isActiveTypeRecursive();

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

    c_time = 0;

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
#ifndef NDEBUG
    for(const node_p &bounding : m_boundaries) {
        assert(bounding != 0);
    }
#endif

    if(level > lvlNoChilds) { // there is still a need of children ;)
        for(size_t i = 0; i < childsByDimension; ++i) {
            createNode(position_t(i));
            m_childs[i]->unpackRecursive(level_t(level - 1));
        }
    }
}

void node_base::updateBackupValueRecursive()
{
    if(isSavetyZone()) {
        updateBackupValue();
    }
    for(node_u &child : m_childs) {
        if(child) {
            child->updateBackupValueRecursive();
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
        updateBackupValue();
    }
}

/*!
 * \brief node_base::interpolation return the interpolation value
 *        based on surrounding elements (only elements with lower level are used)
 * \return the actual value for the interpolation
 */
real node_base::interpolation() const
{
#if 0 // before NDEBUG
    // only in debugging mode interpolation should be called on the boundary
    if(m_level == lvlBoundary) {
        return m_property();
    }
#endif
    assert(m_level > lvlBoundary);
    real property = 0;
    // TODO explicitly unroll this loop?
    for(const node_p &boundary: m_boundaries) {
        property += boundary->property();
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
    , m_propertyBackup(m_property) // TODO do we need it? (obviously yes)
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
    , m_neighbours({{nullptr}})
    , m_center(center)
    , m_property(c_propertyGenerator(center))
    , m_propertyBackup(m_property) // TODO do we need it? (not so expensive)
{
    assert(level < lvlFirst);
}

real node_base::c_epsilon = EPSILON;
real node_base::c_time    = 0;
node_base::node_u node_base::c_root = node_u(nullptr);
node_base::boundaryCondition_t node_base::c_boundaryCondition = node_base::bcIndependent;

node_base::propertyGenerator_t node_base::c_propertyGenerator = node_base::propertyGenerator_t();
