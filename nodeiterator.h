#ifndef NODEITERATOR_H
#define NODEITERATOR_H

#include "node.h"

#include <boost/iterator/iterator_facade.hpp>

class NodeIterator : public boost::iterator_facade<NodeIterator, Node_ptr, boost::forward_traversal_tag>
{
public:
    NodeIterator();
    explicit NodeIterator(const Node_ptr &node) :
        m_node(node) {}
private:
    Node_ptr m_node;
    friend class boost::iterator_core_access;
    /*
    void increment() { m_node = m_node->next(); }

    bool equal(node_iterator const& other) const
    {
        return this->m_node == other.m_node;
    }

    node_base& dereference() const { return *m_node; }
    */
};

#endif // NODEITERATOR_H
