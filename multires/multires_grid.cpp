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
    m_root_node->initialize(nullptr, node_t::lvlRoot, node_t::posRoot, {{}}, m_root_point);
    // create level_start-depth new children
    m_root_node->branch(m_level_start);

    /*
    for(point_t &point: *this) {
        std::cerr << "ind " << point.m_index[0] << " lvl " << point.getLevel(m_level_start) << std::endl;
    }
    */

    // initialize data points and optimize mesh
    size_t size_new = size();
    size_t size_old;
    do {
        size_old = size_new;
        /* FIXME: doesn't work because of wrong implementation of iterator :/
        #pragma omp parallel for
        for(auto point = begin(); point < end(); ++point) {
            point->m_phi = g_f_eval(point->m_x);
            point->m_phiBackup = point->m_phi;
        }
        */
        for(point_t &point: *this) {
            point.m_phi = s_f_eval(point.m_x);
        }
        remesh();
        size_new = size();
        std::cerr << "initalizing: " << size_old << " -> " << size_new << std::endl;
    } while (size_old != size_new);
}

void multires_grid_t::remesh()
{
    m_root_node->remesh_analyse();
    m_root_node->remesh_savety();
    m_root_node->remesh_clean();
}

real multires_grid_t::timeStep()
{
    static u_short counter = 0;
    if (counter % 2 == 0) {
        m_root_node->updateFlow(node_t::posRight);
        m_root_node->timeStep(node_t::posRight);

        m_root_node->updateFlow(node_t::posNorth);
        m_root_node->timeStep(node_t::posNorth);
    } else {
        m_root_node->updateFlow(node_t::posNorth);
        m_root_node->timeStep(node_t::posNorth);

        m_root_node->updateFlow(node_t::posRight);
        m_root_node->timeStep(node_t::posRight);
    }
    ++counter;

    remesh();

    m_time += dt;
    return dt;
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

grid_t::iterator multires_grid_t::begin()
{
    return iterator(m_root_point);
}

grid_t::iterator multires_grid_t::end()
{
    return iterator();
}
