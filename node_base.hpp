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


#include <boost/iterator/iterator_facade.hpp>

// Polymorphic list node base class
// inspired by http://www.boost.org/doc/libs/1_55_0/libs/iterator/example/node.hpp

using boost::logic::tribool;
using boost::logic::indeterminate;

template <class node_derived>
struct node_base
{

    typedef node_base* node_p;
    // http://www.drdobbs.com/cpp/c11-uniqueptr/240002708
    typedef std::unique_ptr<node_base> node_u;

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

    // virtual static node_ptr factory(const node_ptr &parent, Position position, level_t level = 0) = 0;

    node_p increment() const;
    node_p decrement() const;


    inline node_p child(const position_t position) const
    { return m_childs[position].get(); }

    static node_p createRoot(const std::vector<real> &boundary_value)
    {
        assert(boundary_value.size() == childsByDimension);

        c_root = node_u(new node_derived(node_p(nullptr), posRoot, lvlRoot));
        real sum = 0;
        for(size_t i = 0; i < boundary_value.size(); ++i) {
            // TODO these destruction of these edge objects is not properly handled as there are no childs of anything
            node_p edge = node_p(new node_derived(node_p(nullptr), position_t(i), lvlBoundary));
            sum += boundary_value[i];
            edge->setCenter(boundary_value[i]);
            c_root->setBoundary(edge);
        }
        c_root->setCenter(sum/2);
        return c_root.get();
    }

    inline position_t position() const
    { return m_position; }

    inline node_p neighbour(const position_t position) const
    { return m_neighbours[position]; }

    inline void setNeighbour(const node_p node, const position_t position)
    { m_neighbours[position] = node; }

    inline node_p boundary(const position_t position) const
    { return m_boundaries[position]; }

    inline void setBoundary(const node_p node, const position_t position)
    { m_boundaries[position] = node; }

    inline void setBoundary(node_p node)
    {
        this->setBoundary(node, node->position() );
        node->setBoundary(static_cast<node_p>(this), reverse(node->position()));
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

    /*
    tribool active() const
    { return m_active; }

    inline void setActive(tribool active = true)
    { m_active = active; }
    */

    bool pack();

    inline level_t level() const
    { return m_level; }

    inline const node_p& parent() const
    { return m_parent; }

    inline real center(dimension_t dimension = dimX) const
    { return m_center[dimension]; }

    inline void setCenter(real center, dimension_t dimension = dimX)
    { m_center[dimension] = center; }


    void createNode(const position_t position);
    void unpack(const level_t level);

    void detachChild(const position_t position);

    static node_p root()
    { return c_root.get(); }



    // inspired by http://www.boost.org/doc/libs/1_55_0/libs/iterator/example/node.hpp

    class iterator
            : public boost::iterator_facade<
            iterator
            , node_derived
            //, boost::forward_traversal_tag
            , boost::bidirectional_traversal_tag
            >
    {
    public:
        iterator()
            : m_node(nullptr)
        {}

        explicit iterator(node_p p)
            : m_node(p)
        {}

    private:
        friend class boost::iterator_core_access;

        void increment()
        { m_node = m_node->increment(); }

        void decrement()
        { m_node = m_node->decrement(); }

        bool equal(iterator const &other) const
        { return this->m_node == other.m_node; }

        node_derived& dereference() const
        {
            node_derived *blub = static_cast<node_derived*>(m_node);
            return *blub;
        }

        node_p m_node;
    };



    ~node_base();
    node_base(const node_p &parent, position_t position, level_t level);

protected:
    static const position_t direction = posRight;

    node_p m_parent = nullptr;
    const position_t m_position;
    level_t m_level;
    // tribool m_active;

    node_p m_boundaries[childsByDimension] = {nullptr};
    node_p m_neighbours[childsByDimension] = {nullptr};
    node_u m_childs[childsByDimension];

    real   m_center[dimensions];

    static node_u c_root;
};

#endif // NODE_BASE_HPP
