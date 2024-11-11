#pragma once

#include "half_edge.hpp"

class HalfEdgeMesh
{
public:
    using Ptr = std::unique_ptr<HalfEdgeMesh>;

    enum class Preset
    {
        PLANE,
        CUBE,
    };

    static Ptr New() { return std::make_unique<HalfEdgeMesh>(); }
    static Ptr New(Preset preset);
    HalfEdgeMesh() = default;
    ~HalfEdgeMesh() = default;
 
    void EachVertex(const std::function<void(const HalfEdge::Vertex::Ptr&)> &fn) const;
    void EachFace(const std::function<void(const HalfEdge::Face::Ptr&)> &fn) const;
    void EachHalfEdge(const std::function<void(const HalfEdge::Ptr&)> &fn) const;

    Mesh &&GenerateMesh() const;
private:
    std::vector<HalfEdge::Vertex::Ptr> Vertices;
    std::vector<HalfEdge::Face::Ptr> Faces;
    std::vector<HalfEdge::Ptr> Edges;
};