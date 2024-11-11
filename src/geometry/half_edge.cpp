#include "half_edge.hpp"
#include <set>
#include <queue>
#include <map>

HalfEdge::HalfEdge() : Origin(nullptr),
                       Twin(nullptr),
                       IncidentFace(nullptr),
                       Prev(nullptr),
                       Next(nullptr)
{
}

Vec3 HalfEdge::Face::Center() const
{
    Vec3 sum(0);
    auto e = Edge;
    do
    {
        sum += e->Origin->P;
        e = e->Next;
    } while (e != Edge);

    return sum / 3.0f;
}
