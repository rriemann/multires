#ifndef NODE_H
#define NODE_H

#include "settings.h"

#include <vector>
// #include <boost/shared_ptr.hpp> // with c++11 also <memory> provides std::shared_ptr
// #include <boost/enable_shared_from_this.hpp>

#include <memory>
#include <iostream>

using std::enable_shared_from_this;
using std::shared_ptr;

class Node;
typedef shared_ptr<Node> Node_ptr;
// typedef shared_ptr<const Node> Node_const_ptr;
typedef std::vector<Node_ptr> NodeVector;

class Node : public enable_shared_from_this<Node>
{
public:

    enum Position {
        // according to numbering of quadrants, starting with 0
        // http://en.wikipedia.org/wiki/Octant_%28solid_geometry%29
        posRoot = -1,
        // 1D
        posTopRightFront = 0,
        posTopRightBack
        /*
        // 2D extension
        posTopLeftBack,
        posTopLeftFront,
        // 3D extension
        posBottomRightFront,
        posBottomRightBack,
        posBottomLeftBack,
        posBottomLeftFront
        */
    };
    static const unsigned int dimension = DIMENSION;
    static const unsigned int childsByDimension = (1 << DIMENSION);

    static Node_ptr factory(const Node_ptr &parent, Position position, unsigned int level = 0);
    ~Node();
    inline Node_ptr sharedPointer() { return shared_from_this(); }
    inline const Node_ptr &neighbour(const Position position) const { return m_neighbours[position]; }
    inline void setNeighbour(const Position &position, const Node_ptr &node) { m_neighbours[position] = node; }
    // void setNeighbours(const PropertyVector &neighbours);
    // const PropertyVector &neighbors() const { return m_neighbours; }

    void setupChild(const Position position);
    void setupChildren();

    inline void setLevel(const unsigned int &level) { m_level = level; }
    inline void setParent(const Node_ptr &parent) { m_parent = parent; }
    inline real property() const { return m_property; }
    inline void setProperty(real property) { m_property = property; }

private:
    Node(const Node_ptr &parent, Position position, unsigned int level = 0);

    Node_ptr m_parent;
    Position m_position;
    unsigned int m_level;

    Node_ptr m_neighbours[dimension];
    Node_ptr m_childs[childsByDimension];

    real m_property;
};

inline std::ostream& operator<< (std::ostream &stream, const Node &obj) {
    return stream << "(" << obj.property() << ")";
}

#endif // NODE_H
