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

    struct RaycastHit
    {
        HalfEdge::Face::Ptr Face;
        Vec3 Center;

        bool Hit() const { return Face != nullptr; }
    };

public:
    using Ptr = std::shared_ptr<HalfEdgeMesh>;

    static Ptr New() { return std::make_shared<HalfEdgeMesh>(); }
    static Ptr NewPlane();
    static Ptr NewCube();
    HalfEdgeMesh() = default;
    ~HalfEdgeMesh();

    void DeleteFace(const HalfEdge::Face::Ptr &face);
    void Extrude(const HalfEdge::Face::Ptr &fromFace, float distance);

    Mesh GenerateMesh(bool shareVertices = true) const;
    void DebugDrawLine(const std::function<void(const DrawLine &)> &fn, const Vec3 &cameraPosition) const;
    void DebugDrawPoint(const std::function<void(const DrawPoint &)> &fn, const Vec3 &cameraPosition) const;
    void DebugDrawNormal(const std::function<void(const DrawNormal &)> &fn, const Vec3 &cameraPosition) const;
    RaycastHit Raycast(const Ray &ray) const;

private:
    void generateMissingTwins();
    std::vector<HalfEdge::Vertex::Ptr> Vertices;
    std::vector<HalfEdge::Face::Ptr> Faces;
    std::vector<HalfEdge::Ptr> Edges;
};

class HalfEdgeMeshSelection
{
public:
    bool IsSelected() const;
    void Select(const HalfEdge::Face::Ref &face);
    void Clear();

    void DrawLine(const std::function<void(const HalfEdgeMesh::DrawLine &)> &fn, const Vec3 &cameraPosition) const;

public:
    std::vector<HalfEdge::Face::Ref> SelectedFaces;
};