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
class point_t;

class multires_grid_t : public grid_t
{
public:
    multires_grid_t(const u_char level_max, const u_char level_min = 0);

    virtual real timeStep();

    const point_t *begin() const;
    const point_t *end() const;

    virtual ~multires_grid_t();

private:
    multires_grid_t(const multires_grid_t&) = delete; // remove copy constructor

    u_char m_level_max;
    u_char m_level_min;
    node_t *m_root_node;
    point_t *m_root_point;


    friend class node_t;
};

#endif // MULTIRES_GRID_HPP
