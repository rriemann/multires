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
#include "point.hpp"
#include "functions.h"


multires_grid_t::multires_grid_t(const u_char level_max, const u_char level_min, real epsilon)
    : m_level_max(level_max)
    , m_level_min(level_min)
{

    m_root_point = new point_t(g_x0, 0);
    m_root_point->setNext(nullptr);

    node_t::setGrid(this);
    node_t::setEpsilon(epsilon);

    m_root_node = new node_t();
    m_root_node->setPoint(m_root_point);
    m_root_node->initialize(nullptr, node_t::lvlRoot, node_t::posRoot, index_t({{0}}));

    // create level_max-depth new children
    m_root_node->branch(level_max);

    // initialize data points
    for(point_t &point: *this) {
        point.m_phi = f_eval(point.m_x);
        point.m_phiBackup = point.m_phi;
    }

    m_root_node->remesh_analyse();
    m_root_node->remesh_savety();
    m_root_node->remesh_clean();
}

real multires_grid_t::timeStep()
{
    m_time += 1;
    return m_time;
}

size_t multires_grid_t::size() const
{
    return std::distance(begin(), end());
}

const multires_grid_t::iterator multires_grid_t::begin() const
{
    return iterator(m_root_point);
}

const multires_grid_t::iterator multires_grid_t::end() const
{
    return iterator();
}

multires_grid_t::~multires_grid_t()
{
    delete m_root_node;
    delete m_root_point;
}
