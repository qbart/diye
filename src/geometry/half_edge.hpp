#pragma once

#include "../core/all.hpp"
#include "mesh.hpp"
#include <memory>
#include <functional>

class HalfEdge
{
public:
    using Ptr = std::shared_ptr<HalfEdge>;

    struct Face
    {
        using Ptr = std::shared_ptr<Face>;
        static Ptr New() { return std::make_shared<Face>(); }
        Face() : Edge(nullptr) {}

        HalfEdge::Ptr Edge;

        Vec3 Center() const;
    };

    struct Vertex
    {
        using Ptr = std::shared_ptr<Vertex>;
        static Ptr New() { return std::make_shared<Vertex>(); }
        static Ptr New(const Vec3 &v) { return std::make_shared<Vertex>(v); }
        Vertex() : IncidentEdge(nullptr) {}
        Vertex(const Vec3 &v) : IncidentEdge(nullptr), P(v) {}

        Vec3 P;
        HalfEdge::Ptr IncidentEdge;
    };

public:
    static Ptr New() { return std::make_shared<HalfEdge>(); }
    HalfEdge();
    ~HalfEdge() = default;

public:
    Vertex::Ptr Origin;
    Ptr Twin;
    Face::Ptr IncidentFace;
    Ptr Prev;
    Ptr Next;
};