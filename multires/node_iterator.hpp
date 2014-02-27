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

#ifndef NODE_ITERATOR_HPP
#define NODE_ITERATOR_HPP

#include "node_base.hpp"
#include <boost/iterator/iterator_facade.hpp>

// inspired by http://www.boost.org/doc/libs/1_55_0/libs/iterator/example/node.hpp

class node_iterator
        : public boost::iterator_facade<
        node_iterator
        , node_base
        //, boost::forward_traversal_tag
        , boost::bidirectional_traversal_tag
        >
{
public:
    node_iterator()
        : m_node(0)
    {}

    explicit node_iterator(node_base* p)
        : m_node(p)
    {}

private:
    friend class boost::iterator_core_access;

    void increment()
    { m_node = m_node->forward(node_base::c_direction); }

    void decrement()
    { m_node = m_node->forward(node_base::c_reversed ); }

    bool equal(node_iterator const &other) const
    { return this->m_node == other.m_node; }

    node_base& dereference() const
    { return *m_node; }

    node_base* m_node;
};

#endif // NODE_ITERATOR_HPP
