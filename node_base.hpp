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

#ifndef NODE_BASE_HPP
#define NODE_BASE_HPP

#include "settings.h"

#include <iostream>
#include <memory>
#include <cassert>

// Polymorphic list node base class
// inspired by http://www.boost.org/doc/libs/1_55_0/libs/iterator/example/node.hpp

struct node_base;
typedef std::shared_ptr<node_base> node_p;

struct node_base
        : public std::enable_shared_from_this<node_base>
{

    enum position_t {
        // according to numbering of quadrants, starting with 0
        // http://en.wikipedia.org/wiki/Octant_%28solid_geometry%29
          posRoot             =-1
        // 1D
        , posLeft             = 0
        , posRight            = 1
        , posTopRightFront    = 0
        , posTopRightBack     = 1
        // 2D extension
        /*
        , posTopLeftBack      = 2
        , posTopLeftFront     = 3
        */
        // 3D extension
        /*
        , posBottomRightFront = 4
        , posBottomRightBack
        , posBottomLeftBack
        , posBottomLeftFront
        */
    };
    static const unsigned int dimension = DIMENSION;
    static const unsigned int childsByDimension = (1 << DIMENSION);

    // virtual static node_ptr factory(const node_ptr &parent, Position position, uint level = 0) = 0;

    node_p next() const
    {
        return neighbour(direction);
    }

    void print(std::ostream& s) const
    { s << "< level: " << m_level << " pos: " << m_position << " >"; }

    static node_p factory(const node_p &parent, position_t position, uint level = 0)
    { return node_p(new node_base(parent, position, level)); }

    inline position_t position() const
    { return m_position; }

    inline const node_p &neighbour(const position_t position) const
    { return m_neighbours[position]; }

    inline void setNeighbour(const node_p &node, const position_t &position)
    { m_neighbours[position] = node; }

    inline void setNeighbour(node_p &node)
    {
        m_neighbours[node->position()] = node;
        node->setNeighbour(shared_from_this(), reverse(position()));
    }

    static position_t reverse(position_t position)
    {
        // attention: this doesn't make sense for posRoot = -1
        assert(position != posRoot);
        if(position % 2) { // if position is even
            return position_t(position + 1);
        } else {
            return position_t(position - 1);
        }
    }


protected:
    node_base(const node_p &parent, position_t position, uint level = 0)
        : m_parent(parent), m_position(position), m_level(level)
    {}

private:
    static const position_t direction = posRight;

    node_p m_parent;
    position_t m_position;
    uint m_level;

    node_p m_neighbours[dimension];
    node_p m_childs[childsByDimension];
};

inline std::ostream& operator<<(std::ostream& s, node_base const& n)
{
    n.print(s);
    return s;
}

#endif // NODE_BASE_HPP
