/***************************************************************************************
 * Copyright (c) 2014 Robert Riemann <robert@riemann.cc>                                *
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

#ifndef MULTIRES_GRID_HPP
#define MULTIRES_GRID_HPP

#include <iostream>
#include <memory>
#include <cassert>
#include <functional>
#include <boost/format.hpp>

#include "settings.h"
#include "grid.hpp"

class node_t;

/*!
   \brief The multires_grid_t class implements a grid based on multi resolution analysis (MRA)
 */
class multires_grid_t : public grid_t
{
public:
    /*!
       \brief multires_grid_t takes care for setting up a mult resolution grid based on objects node_t
       \param level_max finest level of this grid
       \param level_min coarsest level of this grid
       \param epsilon threshold value to dismiss nodes
     */
    multires_grid_t(const u_char level_max, const u_char level_min = 0, real epsilon = g_epsilon);

    virtual real timeStep(); // documented in grid_t

    void unfold(u_char level_max); //!< creates nodes up to the finest grid to get a regular grid with finest resolution according to m_level_max

    const node_t *getRootNode() const
    { return m_root_node; }

    virtual ~multires_grid_t();

    virtual iterator begin();
    virtual iterator end();

private:
    multires_grid_t(const multires_grid_t&) = delete; // remove copy constructor

    u_char m_level_max; //!< maximum level, finest grid
    u_char m_level_min; //!< minimum level, coarsest grid
    u_char m_level_start; //!< level to start with at initialization
    real dt; //!< global time step
    node_t *m_root_node; //!< pointer to the root node of the underlying tree
    point_t *m_root_point; //!< pointer to the point_t in the lower left edge (root point)

    /*!
       \brief remesh adopts the local granularity of the mesh

       \see node_t::remesh_analyse(), node_t::remesh_savety(), node_t::remesh_clean()
     */
    void remesh();


    friend class node_t;
};


#endif // MULTIRES_GRID_HPP
