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

# include <iostream>
# include <memory>

// Polymorphic list node base class
// inspired by http://www.boost.org/doc/libs/1_55_0/libs/iterator/example/node.hpp

struct node_base;
typedef std::shared_ptr<node_base> node_ptr;

struct node_base : public std::enable_shared_from_this<node_base>
{

    node_base *next() const
    {
        return m_next.get();
    }

    virtual void print(std::ostream& s) const = 0;
    virtual void double_me() = 0;

    void append(node_base* p)
    {
        if (m_next.get())
            m_next->append(p);
        else
            m_next = node_ptr(p);
    }

private:
    node_ptr m_next;
};

inline std::ostream& operator<<(std::ostream& s, node_base const& n)
{
    n.print(s);
    return s;
}

#endif // NODE_BASE_HPP
