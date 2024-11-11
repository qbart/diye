#include "half_edge.hpp"

HalfEdge::HalfEdge(HalfEdge::InitType type)
{
    switch (type)
    {
    case InitType::PLANE:
        
        break;
    case InitType::CUBE:
        break;
    default:
        break;
    }
}

HalfEdge::~HalfEdge()
{
}
