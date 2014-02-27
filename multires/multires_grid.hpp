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
#include <boost/iterator/iterator_facade.hpp>

#include "settings.h"
#include "grid.hpp"
#include "point.hpp"

class node_t;
// class point_t;

class multires_grid_t : public grid_t
{
    class iterator
            : public boost::iterator_facade<
            iterator
            , point_t
            , boost::forward_traversal_tag
            //, boost::bidirectional_traversal_tag
            >
    {
    public:
        iterator()
            : m_point(nullptr)
        {}

        explicit iterator(point_t* p)
            : m_point(p)
        {}

    private:
        friend class boost::iterator_core_access;

        void increment()
        { m_point = m_point->m_next; }

        bool equal(iterator const &other) const
        { return this->m_point == other.m_point; }

        point_t& dereference() const
        { return *m_point; }

        point_t *m_point;
    };

public:
    multires_grid_t(const u_char level_max, const u_char level_min = 0);

    virtual real timeStep();

    const iterator begin() const;
    const iterator end() const;

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
