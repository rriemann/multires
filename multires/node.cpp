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

#include <iostream>

#include "node.hpp"
#include "multires_grid.hpp"
#include "point.hpp"

node_t::node_t()
{
}

void node_t::initialize(node_t *parent, u_char level, char position, const index_t &index)
{
    m_parent = parent;
    m_level = level;
    m_position = position;
    m_index = index;
    m_flags = flUnset;
    m_childs = nullptr;

    if (position > 0) {
        // create new point for position > 0
        // construct point-index
        assert(g_dimension == 1);
        index_t index_p = m_parent->getPoint()->m_index;
        index_p[0] += pow(2, c_grid->m_level_max - m_level);

        real phi = 0;
        for (char pos = 0; pos < c_childs; ++pos) {
            const node_t *neighbour = getNeighbour(pos);
            phi += neighbour->getPoint()->m_phi;
        }
        m_point = new point_t(index_p, c_grid->m_level_max, phi/c_childs);
    }
    /*
    std::cerr << "this pos " << int(position)
              << " this index " << m_point->m_index[0]
              << " this level " << int(m_level)
              << std::endl;
    */

    if(m_level == c_grid->m_level_max) {
        assert(m_index == m_point->m_index);
    }
}
/*!
   \brief node_t::getNeighbour
   \param position to look for the neighbour
   \return the neighbour at position with the same or smaller (coarser) one
*/
const node_t *node_t::getNeighbour(const char position) const
{
    // Check the parent cell's children
    if (m_position == posRoot) {
        return this;
    }

    // m_position can only be right or left
    if (m_position != position) {
        return m_parent->getChild(position);
    }

    const node_t* cnode = m_parent->getNeighbour(position);

    if (cnode->isLeaf()) {
        return cnode;
    } else {
        assert(this == m_parent->getChild(m_position));
        return cnode->getChild((position+1)%2);
    }
}
/*!
   \brief node_t::getChild
   \param position of the child relative to its parent
   \return the child of this node at position

   \note please make sure yourself that this is not a leaf
   \sa isLeaf()
*/
node_t *node_t::getChild(const char position) const
{
    assert(m_childs);
    return &(*m_childs)[position];
}

/*!
   \brief node_t::getChilds
   \return m_childs pointer (might be a nullptr)
*/
node_t::node_array_t *node_t::getChilds() const
{
    // assert(m_childs);
    return m_childs;
}

/*!
   \brief node_t::branch
   \param level gives the number of relative layers to append

   \note this function doesn't check if m_level_max is reached or not
*/
void node_t::branch(size_t level)
{
    if(level > 0) {
        // check if memory is not yet allocated in memory
        if(!m_childs) {
            // allocate memory for all child nodes
            m_childs = new node_array_t;

            // copy point for first child from parent (this)
            getChild(0)->setPoint(m_point);

            for (size_t pos = 0; pos < c_childs; ++pos) {
                // construct node-index
                index_t index = getIndex();
                for (auto &ind: index) {
                    assert(g_dimension == 1);
                    ind = 2*ind+pos;
                }

                getChild(pos)->initialize(this, m_level+1, position_t(pos), index);
                // std::cerr << "now index: " << getChild(pos)->getPoint()->m_index[dimX] << std::endl;

            }

            // put new child nodes into the next-chain of the points
            assert(g_dimension == 1);
            getChild(1)->getPoint()->m_next = m_point->m_next;
            m_point->m_next = getChild(1)->getPoint();
        }
        for (node_t &node: *m_childs) {
            node.branch(level-1);
        }
    }
}

/*!
   \brief node_t::debranch
   \note check if m_childs is non-zero before
 */
void node_t::debranch()
{
    point_t *point = (*m_childs)[c_childs-1].getPoint()->m_next;

    delete m_childs;
    m_childs = nullptr;

    m_point->m_next = point;
}

/*!
   \brief node_t::remesh_analyse
   \return if the current node has flag flActive
*/
bool node_t::remesh_analyse()
{
    if (!has(flCached)) {
        assert(!has(flActive));

        // look for active childs
        if (m_childs) {
            for (node_t &node: *m_childs) {
                if (node.remesh_analyse()) {
                    set(flActive);
                }
            }
        }

        // check if the residual of this node
        if (!has(flActive) && (m_position > 0) && (residual() > c_epsilon)) {
            set(flActive);
        }

        // check neighbours to keep the tree graded
        if (!has(flActive)) {
            /* If there is only one neighbour of my level that has children,
               this node has to stay active.
            */

            for (u_char pos = 0; pos < c_childs; ++pos) {
                const node_t *neighbour = getNeighbour(pos);

                // check if the tree is balanced
                assert(abs(neighbour->getLevel() - m_level) < 2);

                if ((neighbour->getLevel() == m_level) && neighbour->getChilds()) {
                    for (node_t &node: *neighbour->getChilds()) {
                        if (node.remesh_analyse()) {
                            set(flActive);
                            break;
                        }
                    }
                }
            }
        }

        set(flCached);
    }

    return has(flActive);
}

/*!
   \brief node_t::remesh_savety adds savety zone

   This methods walks recursively through the whole tree to find the nodes
   with highest level in each branch which have a flActive flag set and creates
   child nodes (if neccessary) with flSavetyZone flag set.

   Ok, we do it differently. We just branch every node which has the flActive flag
   itself or has a sibling with it.
*/
void node_t::remesh_savety()
{
    if (m_childs && (m_level+1 < c_grid->m_level_max)) {
        // cumulative  flags of children
        u_char cum_flags = flUnset;
        for (node_t &node: *m_childs) {
            node.remesh_savety();
            cum_flags = cum_flags | node.getFlags();
        }
        if (cum_flags & flActive) {
            for (node_t &node: *m_childs) {
                node.branch();
                for (node_t &node: *node.getChilds()) {
                    node.set(flSavetyZone);
                }
            }
        }
    }
}

/*!
   \brief node_t::remesh_clean
   \return if the current node has no children
*/
bool node_t::remesh_clean()
{
    bool veto = false; // veto for removal of this node
    if (m_childs) {
        for (node_t &node: *m_childs) {
            if (!node.remesh_clean()) {
                veto = true;
            }
        }
        if (!veto) {
            debranch();
        }
    }
    bool ret = (!veto && (m_flags < flSavetyZone));
    m_flags = flUnset;
    return ret;
}

real node_t::interpolation() const
{
    assert(m_position != 0);
    real phi = 0;
    for (char pos = 0; pos < c_childs; ++pos) {
        phi += getNeighbour(pos)->getPoint()->m_phi;
    }
    return phi/c_childs;
}

real node_t::residual() const
{
    return fabs(m_point->m_phi - interpolation());
}

void node_t::timeStep()
{
    if(isLeaf()) {
        std::array<const node_t *,  c_childs> neighbours;
        // u_char level_diff_max = 0;
        for (char pos = 0; pos < c_childs; ++pos) {
            neighbours[pos] = getNeighbour(pos);
            /* as we work with graded trees, we can expect that the level of our
           neighbours is either the same or one level smaller (coarser).
        */
            assert(abs(neighbours[pos]->getLevel() - m_level) < 2);
        }

        assert(g_dimension == 1);
        if (m_position == 0) {
            short level_diff = neighbours[1]->getLevel() - neighbours[0]->getLevel();
            if (level_diff > 0) {
                // left node is coarser, we take a coarser neighbour for right as well
                neighbours[1] = m_parent->getNeighbour(1);
            } else if (level_diff < 0) {
                // right node is coarser, we take a coaser neighbour for left as well
                //neighbours[0] = m_parent->getNeighbour(1);
                // automatically fullfiled
            }
        }

        real dxl = m_point->m_x[dimX] - neighbours[posLeft]->getPoint()->m_x[dimX];
        if (m_index[dimX] == 0 ) {
            dxl += g_span[dimX];
        }

#ifndef NDEBUG
        assert(dxl > 0);

        // check if the neighbours have equal distance
        real dxr = neighbours[posRight]->getPoint()->m_x[dimX] - m_point->m_x[dimX];
        if (dxr < 0) {
            dxr += g_span[dimX];
        }

        assert(fabs(dxr-dxl) <= g_eps);
        // check if the neighbours are not too far away
        // std::cerr << "dxr - est. " << dxr << " " << g_span[dimX]/(1 << (m_level-1))+g_eps << std::endl;
        assert(dxl <= g_span[dimX]/(1 << (m_level-1))+g_eps);
#endif
        const real &phi_left  = neighbours[posLeft ]->getPoint()->m_phiBackup;
        const real &phi_right = neighbours[posRight]->getPoint()->m_phiBackup;
        m_point->m_phi = timeStepHelper(m_point->m_phiBackup, phi_left, phi_right, dxl, c_grid->dt);
    } else {
        for (node_t &node: *m_childs) {
            node.timeStep();
        }
    }
}

node_t::~node_t()
{
    if (m_childs) {
        debranch();
    }

    // we delete the position pointers except the one we got from parent
    if(m_position > position_t(0)) {
        delete m_point;
        // m_point = nullptr;
    }
}

multires_grid_t *node_t::c_grid = nullptr;
real node_t::c_epsilon = 0;
