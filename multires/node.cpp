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

    // construct point for non-root and
    //   not the one (pos = 0) which just gets the copy of the parent
    if (position > 0) {
        assert(g_dimension == 1);

        real phi = 0;
        /*
        for (char pos = 0; pos < c_childs; ++pos) {
            const node_t *neighbour = getNeighbour(pos);
            phi += neighbour->getPoint()->m_phi;
        }
        */

        location_t location = {{ parent->getPoint()->m_x[dimX] + g_span[dimX]/(1 << m_level) }};
        m_point = new point_t(location, phi/c_childs);
    }
}

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
            getChild(0)->setPoint(m_point);
            for (size_t pos = 0; pos < c_childs; ++pos) {
                index_t index = m_index;
                for (auto &ind: index) {
                    assert(g_dimension == 1);
                    ind = 2*ind+pos;
                }
                getChild(pos)->initialize(this, m_level+1, position_t(pos), index);

            }

            // put new child nodes into the next-chain of the points
            getChild(1)->getPoint()->m_next = m_point->m_next;
            m_point->m_next = getChild(1)->getPoint();
            // (*m_childs)[0].getPoint()->m_next = (*m_childs)[1].getPoint();
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
    m_flags = m_flags & (~(flActive | flCached));
    return (!veto && (m_flags < flSavetyZone));
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

real node_t::interpolation() const
{
    real phi = 0;
    for (char pos = 0; pos < c_childs; ++pos) {
        const node_t *neighbour = getNeighbour(pos);
        phi += neighbour->getPoint()->m_phi;
    }
    return phi/c_childs;
}

real node_t::residual() const
{
    return fabs(m_point->m_phi - interpolation());
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
