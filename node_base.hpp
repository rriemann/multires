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
#include <boost/format.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/logic/tribool_io.hpp> // also important for correct debug info in gdb

// Polymorphic list node base class
// inspired by http://www.boost.org/doc/libs/1_55_0/libs/iterator/example/node.hpp

struct node_base;
typedef std::shared_ptr<node_base> node_p;
typedef std::weak_ptr<node_base> node_w;
typedef node_p node_tp;
typedef node_base node_t;
using boost::logic::tribool;
using boost::logic::indeterminate;

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
    enum level_t {
          lvlBoundary = -1
        , lvlNoChilds =  0
        , lvlRoot     =  0
    };

    enum dimension_t {
          dimX = 0
    };

    static const unsigned int dimensions = DIMENSION;
    static const unsigned int childsByDimension = (1 << DIMENSION);
    static real epsilon;

    // virtual static node_ptr factory(const node_ptr &parent, Position position, level_t level = 0) = 0;

    inline node_p next() const
    { return neighbour(direction); }

    static node_p factory(const node_p parent, position_t position, level_t level = lvlBoundary)
    { return node_p(new node_base(parent, position, level)); }

    inline position_t position() const
    { return m_position; }

    inline const node_p neighbour(const position_t position) const
    { return m_neighbours[position]; }

    inline void setNeighbour(const node_p node, const position_t position)
    { m_neighbours[position] = node; }

    inline void setNeighbour(node_p node)
    {
        this->setNeighbour(node,                       node->position() );
        node->setNeighbour(shared_from_this(), reverse(node->position()));
    }

    static position_t reverse(position_t position)
    {
        // attention: this doesn't make sense for posRoot = -1
        assert(position != posRoot);
        if(position % 2 == 0) { // if position is even
            return position_t(position + 1);
        } else {
            return position_t(position - 1);
        }
    }

    tribool active() const
    { return m_active; }

    inline void setActive(tribool active = true);

    bool isActive();

    inline level_t level() const
    { return m_level; }

    real property() const
    { return m_property; }

    inline const node_p& parent() const
    { return m_parent; }

    inline real detail() const
    { real detail = (m_property - interpolation()); std::cerr << detail << std::endl; return detail; }

    inline real center(dimension_t dimension = dimX) const
    { return m_center[dimension]; }

    inline void setCenter(real center, dimension_t dimension = dimX)
    { m_center[dimension] = center; m_active = boost::logic::indeterminate; }

    static void setRange(real &span0, real &span1, dimension_t dimension = dimX) {
        c_span0[dimension] = span0;
        c_span1[dimension] = span1;
        c_width[dimension] = span1-span0;
    }


    void setupChild(const position_t position, const level_t level = lvlNoChilds);
    void unpack(const level_t level);

    inline node_p deepNeighbour(const position_t position);
    node_p deepChild(const position_t position);
    void detachChild(const position_t position);
    void detach();

    void pack();
    real interpolation() const;

    real m_property;


protected:
    node_base(const node_p &parent, position_t position, level_t level)
        : m_parent(parent)
        , m_position(position)
        , m_level(level)
        , m_active(boost::logic::indeterminate)
    {
    }

private:
    static const position_t direction = posRight;

    node_p m_parent;
    const position_t m_position;
    level_t m_level;
    tribool m_active;

    node_p m_neighbours[childsByDimension];
    node_p m_childs[childsByDimension];

    real        m_center[dimensions];
    static real c_span0[dimensions];
    static real c_span1[dimensions];
    static real c_width[dimensions];
};

inline std::ostream& operator<<(std::ostream& stream, node_base const& node)
{
    stream << boost::format("< < level: % 2d, pos: % 2d, center: % 1.3f act: %s> property: % 3.3f >") % node.level() % node.position() % node.center() % node.active() % node.property();
    return stream;
}

#endif // NODE_BASE_HPP
