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

#ifndef NODE_REAL_HPP
#define NODE_REAL_HPP

#include "settings.h"
#include "node_base.hpp"

// http://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
class node_real : public node_base<node_real>
{
public:
    inline real property() const
    { return m_property; }

    void setProperty(real property)
    { m_property = property; }

    inline real detail() const
    { return m_property - interpolation();}

    real interpolation() const;

public:
    node_real(const node_p &parent, position_t position, level_t level)
        : node_base(parent, position, level)
    {
        if(level > lvlRoot) {
            m_property = interpolation();
        }
    }

protected:

    real m_property;
};

inline std::ostream& operator<<(std::ostream& stream, node_real const& node)
{
    stream << boost::format("< < level: % 2d, pos: % 2d, center: % 1.3f > property: % 3.3f interpolation: % 3.3f >") % node.level() % node.position() % node.center() % node.property() % node.interpolation();
    return stream;
}

typedef node_real node_t;
typedef node_t* node_tp;

#endif // NODE_REAL_HPP
