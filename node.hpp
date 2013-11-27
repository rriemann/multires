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

#ifndef NODE_HPP
#define NODE_HPP

#include "node_base.hpp"

// inspired by http://www.boost.org/doc/libs/1_55_0/libs/iterator/example/node.hpp

template <class T>
struct node : node_base
{

    void print(std::ostream& s) const
    { s << "< property: " << this->m_property << this->node_base::print(s) << " >" << std::endl; }

    static std::shared_ptr<node> factory(const node_p &parent, position_t position, uint level = 0)
    { return std::shared_ptr<node>(new node(parent, position, level)); }

protected:
    node(const node_p &parent, position_t position, uint level = 0)
        : node_base(parent, position, level)
    {}


private:
    T m_property;
};

typedef node<real> node_t;
typedef std::shared_ptr<node_t> node_tp;

#endif // NODE_HPP
