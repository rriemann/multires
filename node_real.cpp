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

#include "node_real.hpp"


real node_real::interpolation() const
{
#ifndef NO_DEBUG
    // only in debugging mode interpolation should be called on the boundary
    if(m_level == lvlBoundary) {
        return m_property;
    }
#endif
    real property = 0;
    // # TODO explicitly unroll this loop?
    for(size_t i = 0; i < childsByDimension; ++i) {
        property += static_cast<node_real*>(m_boundaries[i])->property();
    }
    return property/childsByDimension;
}
