#ifndef NODE_H
#define NODE_H

#include "settings.h"
#include <vector>
#include <boost/shared_ptr.hpp>

class Property;
typedef boost::shared_ptr<Property> Property_ptr;
typedef std::vector<Property_ptr> PropertyVector;
typedef std::vector<real> RealVector;

class Node
{
public:
    Node();
    void setBoundaries(const PropertyVector& boundaries);
    void setData(RealVector data);

private:
    Property_ptr m_property_ptr;
    PropertyVector m_boundaries;
};

#endif // NODE_H
