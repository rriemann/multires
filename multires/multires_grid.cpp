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

#include "multires_grid.hpp"
#include "node.hpp"


multires_grid_t::multires_grid_t(const size_t level_max, const size_t level_min)
    : m_level_max(level_max)
    , m_level_min(level_min)
{
    node_t::setGrid(this);
    m_root = std::unique_ptr<node_t>(new node_t());
    m_root->initialize(nullptr, node_t::lvlRoot, node_t::posRoot, index_t({{0}}));
    m_root->branch(level_max);
}

multires_grid_t::~multires_grid_t()
{
}
