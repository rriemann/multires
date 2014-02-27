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
        for (char pos = 0; pos < c_childs; ++pos) {
            phi += getNeighbour(pos)->getPoint()->m_phi;
        }

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
        return &(*m_parent->m_childs)[position];
    }

    const node_t* cnode = m_parent->getNeighbour(position);

    if (cnode->isLeaf()) {
        return cnode;
    } else {
        return &(*cnode->m_childs)[position];
    }
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
            (*m_childs)[0].setPoint(m_point);
            for (size_t pos = 0; pos < g_dimension; ++pos) {
                index_t index = m_index;
                for (auto &ind: index) {
                    assert(g_dimension == 1);
                    ind = 2*ind+pos;
                }
                (*m_childs)[pos].initialize(this, m_level+1, position_t(pos), index);

            }
        }
        for (node_t &node: *m_childs) {
            node.branch(level-1);
        }
    }
}

node_t::~node_t()
{
    if(m_childs) {
        delete m_childs;
        // m_childs = nullptr;
    }

    // we delete the position pointers except the one we got from parent
    if(m_position > position_t(0)) {
        delete m_point;
        // m_point = nullptr;
    }
}

multires_grid_t *node_t::c_grid = nullptr;
