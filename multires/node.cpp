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

node_t::node_t()
{
}

void node_t::initialize(node_t *parent, size_t level, position_t position, const index_t &index)
{
    m_parent = parent;
    m_level = level;
    m_position = position;
    m_index = index;
    m_flags = flUnset;
    m_childs = nullptr;
}

const node_t *node_t::getNeighbour(const node_t::position_t position) const
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

void node_t::branch(size_t level)
{
    if (m_level < c_grid->m_level_max) {
        // check if memory is not yet allocated in memory
        if(!m_childs) {
            // allocate memory for all child nodes
            m_childs = new node_array_t;
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
        m_childs = nullptr;
    }
}

multires_grid_t *node_t::c_grid = nullptr;
