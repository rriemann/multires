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

#ifndef POINT_HPP
#define POINT_HPP

#include "settings.h"

class point_t
{
public:
    point_t() {}

    point_t(index_t index, u_char level_max) :
        m_index(index)
    {
        for (u_char i = 0; i < g_dimension; ++i) {
            m_x[i] = g_x0[i] + g_span[0]/(1 << level_max)*m_index[0];
        }
    }

    point_t(index_t index, u_char level_max, field_generator_t f_eval) :
        point_t(index, level_max)
    {
        m_phi = f_eval(m_x);
    }

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

    /*! the index with respect to level_max */
    index_t m_index;
    location_t m_x;
    real m_phi;
    real m_phiBackup;
    point_t *m_next;
};
#endif // POINT_HPP
