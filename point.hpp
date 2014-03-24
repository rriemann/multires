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

#ifndef POINT_HPP
#define POINT_HPP

#include <strings.h> // provides the function ffs()
#include <boost/iterator/iterator_facade.hpp>

#include "settings.h"

/*!
   \brief The point_t class represents one point in the grid—independently of the grid type.
 */
class point_t
{
public:
    point_t() {} //!< empty constructor provided for default-constructing elements in a std container type

    /*!
       \brief constructor that initalizes \ref m_index and the physical position in space \ref m_x
       \param index of this node evaluated in the level level_max
       \param level_max

       \sa g_x0, g_span in settings.h
     */
    point_t(index_t index, const u_char level_max) :
        m_index(index)
    {
        for (u_char i = 0; i < g_dimension; ++i) {
            m_x[i] = g_x0[i] + g_span[i]/(1 << level_max)*m_index[i];
        }
    }

    /*!
       \brief constructor that initializes \ref m_index, he physical position in space \ref m_x and a field value \ref m_phi
       \param index of this node evaluated in the level level_max
       \param level_max
       \param f_eval field generator which is used to initalize \ref m_phi

       \sa point_t(index_t, const u_char)
     */
    point_t(index_t index, u_char level_max, field_generator_t f_eval) :
        point_t(index, level_max)
    {
        m_phi = f_eval(m_x);
    }

    /*!
       \brief constructor that initializes \ref m_index, he physical position in space \ref m_x and a field value \ref m_phi
       \param index of this node evaluated in the level level_max
       \param level_max
       \param value to initialize \ref m_phi

       \sa point_t(index_t, const u_char)
     */
    point_t(index_t index, u_char level_max, real phi) :
        point_t(index, level_max)
    {
        m_phi = phi;
    }

    void setNext(point_t *point)
    { m_next = point; }

    const point_t *getNext() const
    { return m_next; }

    void setIndex(index_t index)
    { m_index = index; }

    /*!
       \brief determines the first level where this point might be part of
       \param level_max is the maximum level on which indexing is based on
       \return the level of this node

       This function is mostly provided for debugging purposes.
     */
    int getLevel(const u_char level_max) const
    {
        int min = (1 << level_max);
        for (const size_t &ind: m_index) {
            int i = ffs(ind);
            if(i < min) min = i;
        }
        if (min == 0) {
            return 0;
        } else {
            return level_max +1 - min;
        }
    }

    index_t m_index; //!< index with respect to level_max in \ref point_t()
    location_t m_x; //!< point location in physical space
    real m_flow; //!< takes the flow calculated by \ref flowHelper()
    real m_phi; //!< actual field variable
    point_t *m_next; //!< part of the forward-only linked list throughout all points in the grid
};
#endif // POINT_HPP
