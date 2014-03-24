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

#ifndef GRID_HPP
#define GRID_HPP

#include "settings.h"
#include "point.hpp"

class grid_t
{
public:
    /*!
       \brief The iterator class is a helper class to iterate over all points part of a multi resolution grid
     */
    class iterator
            : public boost::iterator_facade<
            iterator
            , point_t
            , boost::forward_traversal_tag
            //, boost::bidirectional_traversal_tag
            >
    {
    public:

        explicit iterator(point_t* p = nullptr)
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


    grid_t();

    /*!
       \brief getTime returns the current global time of this grid
       \return returns the current global time of his grid
     */
    real getTime()
    { return m_time; }

    /*!
       \brief timeStep evolves the grid to the next point in time

       The size of the time step might depend on the current configuration of
       the grid. The choosen value is given as a return value.

       \return the size of the time step

       \sa getTime()
     */
    virtual real timeStep() = 0;

    /*!
       \brief size gives back the number of points of type point_t in this grid
       \return the number of points in this grid
     */
    virtual size_t size();

    virtual iterator begin() = 0;
    virtual iterator end() = 0;

protected:
    real m_time = 0; ///< global time
};

#endif // GRID_HPP
