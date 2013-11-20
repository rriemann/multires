#include "node.h"
#include "property.h"

Node::Node()
{
}

void Node::setBoundaries(const PropertyVector &boundaries)
{
    if(m_boundaries.size()) {
        m_boundaries.clear();
    }
    m_boundaries = boundaries;
}

void Node::setData(RealVector data)
{
    // http://www-graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2
    if(((data.size()-1) & (data.size()-2)) != 0) {
        throw "not power of two + 1";
    }
    PropertyVector boundaries;
    boundaries.push_back(Property_ptr(new Property(data[0])));
    boundaries.push_back(Property_ptr(new Property(data[1])));
    setBoundaries(boundaries);
}
