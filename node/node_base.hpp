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
#include <array>
#include <cassert>
#include <boost/format.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/logic/tribool_io.hpp> // also important for correct debug info in gdb

using boost::logic::tribool;
using boost::logic::indeterminate;

// Polymorphic list node base class
// inspired by http://www.boost.org/doc/libs/1_55_0/libs/iterator/example/node.hpp

struct node_base;
typedef node_base* node_p;
// http://www.drdobbs.com/cpp/c11-uniqueptr/240002708
typedef std::unique_ptr<node_base> node_u;
typedef node_p node_tp;
typedef node_base node_t;

struct node_base
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
        , lvlFirst    = 1
    };

    enum dimension_t {
          dimX = 0
    };

    static const unsigned int dimensions = DIMENSION;
    static const unsigned int childsByDimension = (1 << DIMENSION);
    static real epsilon;

    typedef std::array<node_p,childsByDimension> node_p_array;
    typedef std::array<node_u,childsByDimension> node_u_array;

    // virtual static node_ptr factory(const node_ptr &parent, Position position, level_t level = 0) = 0;

    node_p increment() const;
    node_p decrement() const;


    inline node_p child(const position_t position) const
    { return m_childs[position].get(); }

    inline node_u &child(const position_t position)
    { return m_childs[position]; }

    static node_p createRoot(const std::vector<real> &boundary_value);

    inline position_t position() const
    { return m_position; }

    inline node_p neighbour(const position_t position) const
    { return m_neighbours[position]; }

    inline void setNeighbour(const node_p node, const position_t position)
    { m_neighbours[position] = node; }

    inline node_p boundary(const position_t position) const
    {
        return m_boundaries[position];
    }

    static position_t reverse(position_t position)
    {
        // attention: this doesn't make sense for posRoot = -1
        assert(position != posRoot);
        // TODO position_t(position + 1 - (position % 2));
        if(position % 2 == 0) { // if position is even
            return position_t(position + 1);
        } else {
            return position_t(position - 1);
        }
    }

    bool isActive();

    void cleanUp();

    void updateDerivative()
    { m_derivative = (neighbour(direction)->property() - neighbour(reverse(direction))->property())/(neighbour(direction)->center()   - neighbour(reverse(direction))->center()); }

    real derivative() const
    { return m_derivative; }

    void flow();
    void timeStep();

    inline level_t level() const
    { return m_level; }

    real property() const
    { return m_property; }

    inline void setActive(const bool ok = true)
    { m_activeRequirement = ok; }

    inline bool active() const
    { return m_activeRequirement; }

    inline void setVirtual(const bool ok = true)
    { m_virtualRequirement = ok; }

    inline bool isVirtual() const
    { return m_virtualRequirement; }

    inline const node_p& parent() const
    { return m_parent; }

    inline real detail() const
    { real detail = (m_property - interpolation()); return detail; }

    inline real center(dimension_t dimension = dimX) const
    { return m_center[dimension]; }

    inline void setCenter(real center, dimension_t dimension = dimX)
    { m_center[dimension] = center; }


    void createNode(const position_t position);
    void unpack(const level_t level);

    void detachChild(const position_t position);

    real interpolation() const;

    real m_property;
    real m_derivative;

    static node_p root()
    { return c_root.get(); }

    ~node_base();


protected:

private:
    node_base(const node_p &parent, position_t position, level_t level, const node_p_array &boundaries);
    node_base(position_t position, level_t level, const node_p_array &boundaries);
    static const position_t direction = posRight;

    node_p m_parent;
    const position_t m_position;
    level_t m_level;
    /*
    bool m_active = true;
    tribool m_activeChilds = boost::logic::indeterminate;
    tribool m_deletable    = boost::logic::indeterminate;
    */

    bool m_cached = false;
    bool m_virtualRequirement = false;
    bool m_activeRequirement  = false;

    const node_p_array m_boundaries; // TODO make it const?
    node_p_array m_neighbours = {}; // initalize nullptr
    node_u_array m_childs;

    real   m_center[dimensions];

    static node_u c_root;
};

inline std::ostream& operator<<(std::ostream& stream, node_base const& node)
{
    stream << boost::format("< < level: % 3d, pos: % 2d, act: % 01d, center: % 1.3f > property: % 3.3f interpolation: % 3.3f >") % node.level() % node.position() % (int(node.active()) + int(node.isVirtual())) % node.center() % node.property() % node.interpolation();
    return stream;
}

#endif // NODE_BASE_HPP
