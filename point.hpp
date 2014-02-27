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
    point_t();

    point_t(location_t x, real phi = 0) :
        m_x(x)
      , m_phi(phi)
      , m_phiBackup(phi)
    {
    }

    index_t m_J = {};
    location_t m_x = {};
    real m_phi = 0;
    real m_phiBackup = 0;
    point_t *next = nullptr;
};
#endif // POINT_HPP
