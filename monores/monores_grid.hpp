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

#ifndef MONORES_GRID_HPP
#define MONORES_GRID_HPP

#include "settings.h"

#include "grid.hpp"
#include "point.hpp"

class monores_grid_t : public grid_t
{
public:
    monores_grid_t(const u_char level_max);

    virtual real timeStep();

    size_t size() const
    { return N; }

    // getters for ranged for, see:
    // http://stackoverflow.com/questions/8164567/how-to-make-my-custom-type-to-work-with-range-based-for-loops
    std::vector<point_t>::iterator begin();
    std::vector<point_t>::iterator end();

    virtual ~monores_grid_t() {}

private:
    monores_grid_t(const monores_grid_t&) = delete; // remove copy constructor



    const size_t N;
    const real dx;
    const real dt;

    std::vector<point_t> pointvector;
};

#endif // MONORES_GRID_HPP
