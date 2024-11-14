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
    return IsPolygon(3);
}

inline bool HalfEdge::Face::IsQuad() const
{
    return IsPolygon(4);
}

inline bool HalfEdge::Face::IsPolygon(int n) const
{
    auto e = Edge;
    int count = 0;
    do
    {
        e = e->Next;
        ++count;
    } while (e != Edge);

    if (n == -1)
        return count >= 5;
    
    if (n == count)
        return true;

    return false;
}

void HalfEdge::Face::EachTriangle(std::function<void(const Vertex::Ptr &a, const Vertex::Ptr &b, const Vertex::Ptr &c)> fn)
{
    if (IsTriangle())
    {
        fn(Edge->Origin, Edge->Next->Origin, Edge->Next->Next->Origin);
    }
    else if (IsQuad())
    {
        auto a = Edge->Origin;
        auto b = Edge->Next->Origin;
        auto c = Edge->Next->Next->Origin;
        auto d = Edge->Next->Next->Next->Origin;

        fn(a, b, c);
        fn(a, c, d);
    }
    else
    {
        auto e = Edge;
        do
        {
            fn(e->Origin, e->Next->Origin, e->Next->Next->Origin);
            e = e->Next->Next;
        } while (e != Edge);
    }
}
