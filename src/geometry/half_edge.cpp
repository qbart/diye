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
    float points = 0;
    do
    {
        sum += e->Origin->P;
        e = e->Next;
        points += 1;
    } while (e != Edge);

    return sum / points;
}

Vec3 HalfEdge::Face::Normal() const
{
    auto edge1 = Edge->Next->Origin->P - Edge->Origin->P; // v1 - v0
    auto edge2 = Edge->Next->Next->Origin->P - Edge->Origin->P; // v2 - v0
    return Mathf::Normalize(Mathf::Cross(edge1, edge2));
}

inline bool HalfEdge::Face::IsTriangle() const
{
    return IsNgon(3);
}

inline bool HalfEdge::Face::IsQuad() const
{
    return IsNgon(4);
}

inline bool HalfEdge::Face::IsPolygon() const
{
    auto e = Edge;
    int count = 0;
    do
    {
        e = e->Next;
        ++count;
        if (count > 4)
            return true;
    } while (e != Edge);

    return false;
}

inline bool HalfEdge::Face::IsNgon(int n) const
{
    auto e = Edge;
    int count = 0;
    do
    {
        e = e->Next;
        ++count;
    } while (e != Edge);

    return count == n;
}
