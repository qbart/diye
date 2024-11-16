#pragma once

#include "half_edge.hpp"

class HalfEdgeMesh
{
public:
    struct DrawLine
    {
        Vec3 From;
        Vec3 To;
        bool Boundary;
        bool Visible;
    };
    struct DrawPoint
    {
        Vec3 Position;
        bool Center;
        bool Visible;
    };
    struct DrawNormal
    {
        Vec3 From;
        Vec3 Direction;
        bool Visible;
    };

public:
    using Ptr = std::unique_ptr<HalfEdgeMesh>;

    static Ptr New() { return std::make_unique<HalfEdgeMesh>(); }
    static Ptr NewPlane();
    static Ptr NewCube();
    HalfEdgeMesh() = default;
    ~HalfEdgeMesh() = default;

    const std::vector<HalfEdge::Face::Ptr> &GetFaces() const { return Faces; }

    Mesh GenerateMesh(bool shareVertices = true) const;
    void DebugDrawLine(const std::function<void(const DrawLine &)> &fn, const Vec3 &cameraPosition) const;
    void DebugDrawPoint(const std::function<void(const DrawPoint &)> &fn, const Vec3 &cameraPosition) const;
    void DebugDrawNormal(const std::function<void(const DrawNormal &)> &fn, const Vec3 &cameraPosition) const;

private:
    void generateMissingTwins();
    std::vector<HalfEdge::Vertex::Ptr> Vertices;
    std::vector<HalfEdge::Face::Ptr> Faces;
    std::vector<HalfEdge::Ptr> Edges;
};