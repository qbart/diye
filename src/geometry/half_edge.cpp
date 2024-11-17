#include "half_edge.hpp"
#include <set>
#include <queue>
#include <map>

HalfEdge::HalfEdge()
{
    Origin.reset();
    Twin.reset();
    IncidentFace.reset();
    Prev.reset();
    Next.reset();
}

HalfEdge::~HalfEdge()
{
    Origin.reset();
    Twin.reset();
    IncidentFace.reset();
    Prev.reset();
    Next.reset();

    fmt::println("HalfEdge deleted");
}

HalfEdge::Vertex::~Vertex()
{
    IncidentEdge.reset();
    fmt::println("Vertex deleted");
}

HalfEdge::Face::~Face()
{
    Edge.reset();
    fmt::println("Face deleted");
}

Vec3 HalfEdge::Face::Center() const
{
    Vec3 sum(0);
    auto e = Edge.lock();
    float points = 0;
    do
    {
        sum += e->Origin.lock()->P;
        e = e->Next.lock();
        points += 1;
    } while (e != Edge.lock());

    return sum / points;
}

Vec3 HalfEdge::Face::Normal() const
{
    auto edge1 = Edge.lock()->Next.lock()->Origin.lock()->P - Edge.lock()->Origin.lock()->P;       // v1 - v0
    auto edge2 = Edge.lock()->Next.lock()->Next.lock()->Origin.lock()->P - Edge.lock()->Origin.lock()->P; // v2 - v0
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
    auto e = Edge.lock();
    int count = 0;
    do
    {
        e = e->Next.lock();
        ++count;
    } while (e != Edge.lock());

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
        fn(Edge.lock()->Origin.lock(), Edge.lock()->Next.lock()->Origin.lock(), Edge.lock()->Next.lock()->Next.lock()->Origin.lock());
    }
    else if (IsQuad())
    {
        auto a = Edge.lock()->Origin.lock();
        auto b = Edge.lock()->Next.lock()->Origin.lock();
        auto c = Edge.lock()->Next.lock()->Next.lock()->Origin.lock();
        auto d = Edge.lock()->Next.lock()->Next.lock()->Next.lock()->Origin.lock();

        fn(a, b, c);
        fn(a, c, d);
    }
    else
    {
        auto e = Edge.lock();
        do
        {
            fn(e->Origin.lock(), e->Next.lock()->Origin.lock(), e->Next.lock()->Next.lock()->Origin.lock());
            e = e->Next.lock()->Next.lock();
        } while (e != Edge.lock());
    }
}
