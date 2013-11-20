#ifndef CELL_H
#define CELL_H

#include "settings.h"

class Property
{
public:
    Property(real value = 0);
    inline real value() const { return m_value; }
    void setValue(real value);
private:
    real m_value;
};


#endif // CELL_H
