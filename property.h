#ifndef CELL_H
#define CELL_H

#include "settings.h"

#include <vector>
// #include <boost/shared_ptr.hpp> // with c++11 also <memory> provides std::shared_ptr
// #include <boost/enable_shared_from_this.hpp>
#include <memory>

using std::enable_shared_from_this;
using std::shared_ptr;

class Property;
typedef shared_ptr<Property> Property_ptr;
typedef std::vector<Property_ptr> PropertyVector;

class Property : public enable_shared_from_this<Property>
{
public:
    Property(real value);
    Property_ptr sharedPointer() { return shared_from_this(); }
    inline real value() const { return m_value; }
    void setValue(real value);
private:
    real m_value;
};


#endif // CELL_H
