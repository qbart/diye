#pragma once

#include "../core/all.hpp"
#include "mesh.hpp"
#include <memory>
#include <functional>

class HalfEdge
{
public:
    using Ptr = std::shared_ptr<HalfEdge>;
    using Ref = std::weak_ptr<HalfEdge>;

    struct Vertex
    {
        using Ptr = std::shared_ptr<Vertex>;
        using Ref = std::weak_ptr<Vertex>;
        static Ptr New() { return std::make_shared<Vertex>(); }
        static Ptr New(const Vec3 &v) { return std::make_shared<Vertex>(v); }
        Vertex() { IncidentEdge.reset(); }
        Vertex(const Vec3 &v) : P(v) { IncidentEdge.reset(); }
        ~Vertex();

        Vec3 P;
        HalfEdge::Ref IncidentEdge;
    };

    struct Face
    {
        using Ptr = std::shared_ptr<Face>;
        using Ref = std::weak_ptr<Face>;
        static Ptr New() { return std::make_shared<Face>(); }
        Face() { Edge.reset(); }
        ~Face();

        HalfEdge::Ref Edge;

        Vec3 Center() const;
        Vec3 Normal() const;
        inline bool IsTriangle() const;          // 3
        inline bool IsQuad() const;              // 4
        inline bool IsPolygon(int n = -1) const; // >= 5
        void EachTriangle(std::function<void(const Vertex::Ptr &a, const Vertex::Ptr &b, const Vertex::Ptr &c)> fn);
    };

public:
    static Ptr New() { return std::make_shared<HalfEdge>(); }
    HalfEdge();
    ~HalfEdge();

public:
    Vertex::Ref Origin;
    Ref Twin;
    Face::Ref IncidentFace;
    Ref Prev;
    Ref Next;
};