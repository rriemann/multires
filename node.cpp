#include "node.h"
#include "property.h"

#include <assert.h>

/*
void Node::setNeighbours(const PropertyVector &neighbours)
{
    if(m_neighbours.size()) {
        m_neighbours.clear();
    }
    m_neighbours = neighbours;
}
*/

void Node::setupChild(const Node::Position position)
{
    m_childs[position] = factory(shared_from_this(), position, m_level-1);

    // the childs gets inserted between this nodes boundary and this nodes own value

    // inherit the boundary value of this parent to the child node
    m_childs[position]->setNeighbour(position, this->neighbour(position));
    // set the boundary value of this parent value to the childs property
    this->setNeighbour(position, m_childs[position]->property());
}


void Node::setupChildren()
{
    if(m_level > 0) {
        assert(dimension == 1); // only support 1D
        // child front

        setupChild(posTopRightFront);
        setupChild(posTopRightBack );
    }
    for(size_t i = 0; i < childsByDimension; ++i) {
        if(m_childs[i].get()) {
            m_childs[i]->setupChildren();
        }
    }
}

Node::Node(const Node_ptr &parent, Node::Position position, unsigned int level) :
    m_parent(parent), m_position(position), m_level(level)
{
    // unused
}

Node_ptr Node::factory(const Node_ptr &parent, Position position, unsigned int level)
{
    Node_ptr pointer(new Node(parent, position, level));
    return pointer;
}

Node::~Node() {
    // give back the edge property to parent
    if(m_position != posRoot) { // if parent exists (this is not root)
        m_parent->setNeighbour(m_position, m_property);
    }
}
