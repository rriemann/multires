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
    , m_level_start((level_max+level_min)/2)
    , dt(g_cfl*g_span[dimX]/((1 << level_max)*g_velocity))
{

    m_root_point = new point_t({{}}, m_level_max);
    assert(m_root_point->m_index[0] == 0);
    m_root_point->setNext(nullptr);

    node_t::setGrid(this);
    node_t::setEpsilon(epsilon);

    m_root_node = new node_t();
    m_root_node->setPoint(m_root_point);
    m_root_node->initialize(nullptr, node_t::lvlRoot, node_t::posRoot, {{}});
    // create level_start-depth new children
    m_root_node->branch(m_level_start);

    // initialize data points and optimize mesh
    size_t size_new = size();
    size_t size_old;
    do {
        size_old = size_new;
        for(point_t &point: *this) {
            point.m_phi = g_f_eval(point.m_x);
            point.m_phiBackup = point.m_phi;
        }
        m_root_node->remesh_analyse();
        m_root_node->remesh_savety();
        m_root_node->remesh_clean();
        size_new = size();
        std::cerr << "initalizing: " << size_old << " -> " << size_new << std::endl;
    } while (size_old != size_new);
}

real multires_grid_t::timeStep()
{
    // update temporary data;
    for(point_t &point: *this) {
        point.m_phiBackup = point.m_phi;
    }

    m_root_node->timeStep();

    m_time += dt;
    return dt;
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

void multires_grid_t::unfold(u_char level_max)
{
    m_root_node->branch(level_max);
}

multires_grid_t::~multires_grid_t()
{
    delete m_root_node;
    delete m_root_point;
}
