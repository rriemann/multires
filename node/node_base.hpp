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
#include <functional>
#include <boost/format.hpp>

// Polymorphic list node base class
// inspired by http://www.boost.org/doc/libs/1_55_0/libs/iterator/example/node.hpp

struct node_base;

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

    // this is designed to allow: if(type>typeVirtual) { eval(); }
    enum type_t {
          typeUnset      = 0
        , typeCached     = 1 << 0
        , typeDeletable  = 1 << 1
        , typeVirtual    = 1 << 2
        , typeSavetyZone = 1 << 3
        , typeActive     = 1 << 4
    };

    static const unsigned int childsByDimension = (1 << g_dimension);
    static real c_epsilon;
    static real c_time;
    static real c_timestep;
    static level_t c_currentmaxlevel;
    static level_t c_maxlevel;

    static const position_t c_direction = posRight;
    static const position_t c_reversed  = posLeft;

    // http://www.drdobbs.com/cpp/c11-uniqueptr/240002708
    typedef node_base* node_p;
    typedef std::unique_ptr<node_base> node_u;
    typedef std::array<node_p,childsByDimension> node_p_array;
    typedef std::array<node_u,childsByDimension> node_u_array;

    // virtual static node_ptr factory(const node_ptr &parent, Position position, level_t level = 0) = 0;

    /*! cycle through the tree without crossing boundary */
    node_p forward(const position_t position) const;

    inline node_p child(const position_t position) const
    { return m_childs[position].get(); }

    inline node_u &child(const position_t position)
    { return m_childs[position]; }

    inline const node_u_array &childs() const
    { return m_childs; }

    static node_p createRoot(const std::vector<real> &boundary_value, const propertyGenerator_t &propertyGenerator, level_t level = level_t(g_level), const boundaryCondition_t boundaryCondition = bcPeriodic, const bool auto_timestep = true);

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

    static real getTime()
    { return c_time; }

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

    bool isActiveTypeRecursive();

    inline void updateBackupValue()
    { m_propertyBackup = m_property; }

    inline void updateTheoryValue()
    {
        realarray center = m_center;
        // FIXME TODO : dublication, see theory_base.hpp
        // we have to use fmod here again because inDomain() is buggy
        center[0] = inDomain(center[0]-std::fmod(c_time*g_velocity, g_span));
        assert(center[0] >= x0 && center[0] <= x1);
        m_propertyTheory = c_propertyGenerator(center);
    }

    real timeStep();
    void optimizeTree();

    inline level_t level() const
    { return m_level; }

    real property() const
    { return m_property; }

    real propertyBackup() const
    { return m_propertyBackup; }

    real propertyTheory() const
    { return m_propertyTheory; }

    inline void set(type_t type)
    { m_type = type_t(m_type | type); }

    inline bool is(type_t type) const
    { return m_type & type; }

    inline void setActive()
    { set(typeActive); }

    inline bool isActive() const
    { return m_type >= typeActive; }

    inline void setSavetyZone()
    { set(typeSavetyZone); }

    inline bool isSavetyZone() const
    { return m_type >= typeSavetyZone; }

    inline void setVirtual()
    { set(typeVirtual); }

    inline bool isVirtual() const
    { return m_type >= typeVirtual; }

    inline type_t type() const
    { return m_type; }

    inline const node_p& parent() const
    { return m_parent; }

    inline real detail() const
    { real detail = (m_property - interpolation()); return detail; }

    inline realarray center() const
    { return m_center; }

    inline real center(dimension_t dimension) const
    { return m_center[dimension]; }

    real interpolation() const;

    static node_p root()
    { return c_root.get(); }

    // for testing purposes only
    void unpackRecursiveTesting(const level_t level)
    { unpackRecursive(level); }

    static void setEpsilon(real epsilon)
    { c_epsilon = epsilon; }

    ~node_base();


protected:
    real timeStepValue();

private:
    node_base(const node_base&) = delete; // remove copy constructor
    node_base(const node_p &parent, position_t position, level_t level, const node_p_array &boundaries);
    node_base(realarray center, position_t position, level_t level, const node_p_array &boundaries);

    void createNode(const position_t position);

    void unpackRecursive(const level_t level);
    static real getdt(const real velocity, const size_t level = c_maxlevel);
    void updateBackupValueRecursive();
    void cleanUpRecursive();
    void initPropertyRecursive();
    void timeStepRecursive();

    node_p m_parent;
    const position_t m_position;
    level_t m_level;

    type_t m_type = typeUnset;

    const node_p_array m_boundaries; // TODO make it const?
    node_p_array m_neighbours = {}; // initalize nullptr
    node_u_array m_childs;

    realarray   m_center;

    static node_u c_root;

    static propertyGenerator_t c_propertyGenerator;

    static boundaryCondition_t c_boundaryCondition;

    static bool c_auto_timestep;

public:
    real m_property;
    real m_propertyBackup;
    real m_propertyTheory;

#ifdef BURGER
    const real &m_speed = m_propertyBackup;
#else
    const real &m_speed = g_velocity;
#endif
};

inline std::ostream& operator<<(std::ostream& stream, node_base const& node)
{
    stream << boost::format("< < level: % 3d, pos: % 2d, act: % 01d, center: % 1.3f > property: % 3.3f interpolation: % 3.3f >") % node.level() % node.position() % (int(node.isActive()) + int(node.isVirtual())) % node.center(dimX) % node.property() % node.interpolation();
    return stream;
}

typedef node_base::node_p node_tp;
typedef node_base node_t;

#endif // NODE_BASE_HPP
