#include <iostream>

// using namespace std;

#include "property.h"
#include "node.h"

int main()
{
    // RealVector data = {0, 1, 2, 3, 4, 5, 6, 7, 8};
    Node_ptr no_parent(NULL);
    Node_ptr root = Node::factory(no_parent, Node::posRoot, 1);
    Property_ptr front(new Property(0));
    root->setNeighbour(Node::posTopRightFront, front);
    Property_ptr back (new Property(1));
    root->setNeighbour(Node::posTopRightBack,  back);
    root->setupChildren();
    return 0;
}
