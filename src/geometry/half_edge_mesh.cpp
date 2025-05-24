#include "half_edge_mesh.hpp"
#include <algorithm>
#include <map>

HalfEdgeMesh::Ptr HalfEdgeMesh::NewPlane()
{
    using Vertex = HalfEdge::Vertex;
    using Face = HalfEdge::Face;

    HalfEdge::Ptr e[8];
    Face::Ptr f[1];
    Vertex::Ptr v[4];

    v[0] = Vertex::New({0, 0, 1});
    v[1] = Vertex::New({1, 0, 1});
    v[2] = Vertex::New({1, 0, 0});
    v[3] = Vertex::New({0, 0, 0});

    for (int i = 0; i < 1; ++i)
        f[i] = Face::New();

    for (int i = 0; i < 8; ++i)
        e[i] = HalfEdge::New();

    // https://jerryyin.info/geometry-processing-algorithms/half-edge/
    // Half-edge	Origin	Twin	Incident face	Next	Prev
    // e0	v0	e6	f0	e1	e2
    // e1	v2	e7	f0	e2	e0
    // e2	v3	e3	f0	e0	e1
    // e3	v0	e2	f1	e4	e5
    // e4	v3	e8	f1	e5	e3
    // e5	v1	e9	f1	e3	e4
    // e6	v2	e0	∅	e9	e7
    // e7	v3	e1	∅	e6	e8
    // e8	v1	e4	∅	e7	e9
    // e9	v0	e5	∅	e8	e6

    // Vertex	Coordinate	Incident edge
    // v1	(0, 1, 0)	e0
    // v2	(1, 1, 0)	e5
    // v3	(0, 0, 0)	e1
    // v4	(1, 0, 0)	e2

    f[0]->Edge = e[0];

    v[0]->IncidentEdge = e[0];
    v[1]->IncidentEdge = e[1];
    v[2]->IncidentEdge = e[2];
    v[3]->IncidentEdge = e[3];

    e[0]->Origin = v[0];
    e[0]->IncidentFace = f[0];
    e[0]->Next = e[1];
    e[0]->Prev = e[3];

    e[1]->Origin = v[1];
    e[1]->IncidentFace = f[0];
    e[1]->Next = e[2];
    e[1]->Prev = e[0];

    e[2]->Origin = v[2];
    e[2]->IncidentFace = f[0];
    e[2]->Next = e[3];
    e[2]->Prev = e[1];

    e[3]->Origin = v[3];
    e[3]->IncidentFace = f[0];
    e[3]->Next = e[0];
    e[3]->Prev = e[2];

    e[4]->Origin = v[0];
    e[4]->IncidentFace.reset();
    e[4]->Next = e[5];
    e[4]->Prev = e[7];

    e[5]->Origin = v[3];
    e[5]->IncidentFace.reset();
    e[5]->Next = e[6];
    e[5]->Prev = e[4];

    e[6]->Origin = v[2];
    e[6]->IncidentFace.reset();
    e[6]->Next = e[7];
    e[6]->Prev = e[5];

    e[7]->Origin = v[1];
    e[6]->IncidentFace.reset();
    e[7]->Next = e[4];
    e[7]->Prev = e[6];

    auto mesh = std::make_shared<HalfEdgeMesh>();
    for (int i = 0; i < 4; ++i)
        mesh->Vertices.emplace_back(v[i]);

    for (int i = 0; i < 1; ++i)
        mesh->Faces.emplace_back(f[i]);

    for (int i = 0; i < 8; ++i)
        mesh->Edges.emplace_back(e[i]);

    mesh->generateMissingTwins();

    return mesh;
}

HalfEdgeMesh::Ptr HalfEdgeMesh::NewCube()
{
    using Vertex = HalfEdge::Vertex;
    using Face = HalfEdge::Face;

    HalfEdge::Ptr e[24]; // 24 half-edges for a cube
    Face::Ptr f[6];      // 6 faces for a cube
    Vertex::Ptr v[8];    // 8 vertices for a cube

    // Define 8 vertices (positions for a unit cube)
    v[0] = Vertex::New({0, 0, 1});
    v[1] = Vertex::New({1, 0, 1});
    v[2] = Vertex::New({1, 1, 1});
    v[3] = Vertex::New({0, 1, 1});
    v[4] = Vertex::New({0, 0, 0});
    v[5] = Vertex::New({1, 0, 0});
    v[6] = Vertex::New({1, 1, 0});
    v[7] = Vertex::New({0, 1, 0});

    // Create 6 faces
    for (int i = 0; i < 6; ++i)
        f[i] = Face::New();

    // Create 24 half-edges
    for (int i = 0; i < 24; ++i)
        e[i] = HalfEdge::New();

    // Front face (f0)
    f[0]->Edge = e[0];
    e[0]->IncidentFace = f[0];
    e[1]->IncidentFace = f[0];
    e[2]->IncidentFace = f[0];
    e[3]->IncidentFace = f[0];
    e[0]->Origin = v[0];
    e[1]->Origin = v[1];
    e[2]->Origin = v[2];
    e[3]->Origin = v[3];
    e[0]->Next = e[1];
    e[1]->Next = e[2];
    e[2]->Next = e[3];
    e[3]->Next = e[0];
    e[0]->Prev = e[3];
    e[1]->Prev = e[0];
    e[2]->Prev = e[1];
    e[3]->Prev = e[2];

    // left face (f1)
    f[1]->Edge = e[4];
    e[4]->IncidentFace = f[1];
    e[5]->IncidentFace = f[1];
    e[6]->IncidentFace = f[1];
    e[7]->IncidentFace = f[1];
    e[4]->Origin = v[0];
    e[5]->Origin = v[3];
    e[6]->Origin = v[7];
    e[7]->Origin = v[4];
    e[4]->Next = e[5];
    e[5]->Next = e[6];
    e[6]->Next = e[7];
    e[7]->Next = e[4];
    e[4]->Prev = e[7];
    e[5]->Prev = e[4];
    e[6]->Prev = e[5];
    e[7]->Prev = e[6];

    // right face (f2)
    f[2]->Edge = e[8];
    e[8]->IncidentFace = f[2];
    e[9]->IncidentFace = f[2];
    e[10]->IncidentFace = f[2];
    e[11]->IncidentFace = f[2];
    e[8]->Origin = v[1];
    e[9]->Origin = v[5];
    e[10]->Origin = v[6];
    e[11]->Origin = v[2];
    e[8]->Next = e[9];
    e[9]->Next = e[10];
    e[10]->Next = e[11];
    e[11]->Next = e[8];
    e[8]->Prev = e[11];
    e[9]->Prev = e[8];
    e[10]->Prev = e[9];
    e[11]->Prev = e[10];

    // back face (f3)
    f[3]->Edge = e[12];
    e[12]->IncidentFace = f[3];
    e[13]->IncidentFace = f[3];
    e[14]->IncidentFace = f[3];
    e[15]->IncidentFace = f[3];
    e[12]->Origin = v[5];
    e[13]->Origin = v[4];
    e[14]->Origin = v[7];
    e[15]->Origin = v[6];
    e[12]->Next = e[13];
    e[13]->Next = e[14];
    e[14]->Next = e[15];
    e[15]->Next = e[12];
    e[12]->Prev = e[15];
    e[13]->Prev = e[12];
    e[14]->Prev = e[13];
    e[15]->Prev = e[14];

    // top face (f4)
    f[4]->Edge = e[16];
    e[16]->IncidentFace = f[4];
    e[17]->IncidentFace = f[4];
    e[18]->IncidentFace = f[4];
    e[19]->IncidentFace = f[4];
    e[16]->Origin = v[3];
    e[17]->Origin = v[2];
    e[18]->Origin = v[6];
    e[19]->Origin = v[7];
    e[16]->Next = e[17];
    e[17]->Next = e[18];
    e[18]->Next = e[19];
    e[19]->Next = e[16];
    e[16]->Prev = e[19];
    e[17]->Prev = e[16];
    e[18]->Prev = e[17];
    e[19]->Prev = e[18];

    // bottom face (f5)
    f[5]->Edge = e[20];
    e[20]->IncidentFace = f[5];
    e[21]->IncidentFace = f[5];
    e[22]->IncidentFace = f[5];
    e[23]->IncidentFace = f[5];
    e[20]->Origin = v[0];
    e[21]->Origin = v[4];
    e[22]->Origin = v[5];
    e[23]->Origin = v[1];
    e[20]->Next = e[21];
    e[21]->Next = e[22];
    e[22]->Next = e[23];
    e[23]->Next = e[20];
    e[20]->Prev = e[23];
    e[21]->Prev = e[20];
    e[22]->Prev = e[21];
    e[23]->Prev = e[22];

    // Set the incident edges for vertices
    v[0]->IncidentEdge = e[0];
    v[1]->IncidentEdge = e[1];
    v[2]->IncidentEdge = e[2];
    v[3]->IncidentEdge = e[3];
    v[4]->IncidentEdge = e[12];
    v[5]->IncidentEdge = e[13];
    v[6]->IncidentEdge = e[14];
    v[7]->IncidentEdge = e[15];

    // Create the mesh
    auto mesh = std::make_shared<HalfEdgeMesh>();
    for (int i = 0; i < 8; ++i)
        mesh->Vertices.emplace_back(v[i]);

    for (int i = 0; i < 6; ++i)
        mesh->Faces.emplace_back(f[i]);

    for (int i = 0; i < 24; ++i)
        mesh->Edges.emplace_back(e[i]);

    mesh->generateMissingTwins();

    return mesh;
}

HalfEdgeMesh::~HalfEdgeMesh()
{
    fmt::println("HalfEdgeMesh deleted");
    Faces.clear();
    Vertices.clear();
    Edges.clear();
}

void HalfEdgeMesh::DeleteFace(const HalfEdge::Face::Ptr &face)
{
    auto e = face->Edge.lock();
    std::vector<HalfEdge::Ptr> edges;
    std::vector<HalfEdge::Vertex::Ptr> orphanVertices;
    do
    {
        edges.emplace_back(e);
        auto twin = e->Twin.lock();
        if (twin)
        {
            twin->Twin.reset();
            auto isBoundary = twin->IncidentFace.expired();
            if (isBoundary)
            {
                edges.emplace_back(twin);
            }
        }
        e->IncidentFace.reset();
        e = e->Next.lock();
    } while (e != face->Edge.lock());

    e = face->Edge.lock();
    do
    {
        auto vertex = e->Origin.lock();
        auto edge = vertex->IncidentEdge.lock();
        if (edge->IncidentFace.expired())
        {
            orphanVertices.emplace_back(vertex);
        }
        e->Origin.lock()->IncidentEdge.reset();
        e->Origin.reset();
        e = e->Next.lock();
    } while (e != face->Edge.lock());

    face->Edge.reset();

    fmt::println("Faces: {}, Edges: {}, Vertex: {}", Faces.size(), Edges.size(), Vertices.size());
    Faces.erase(std::remove(Faces.begin(), Faces.end(), face), Faces.end());
    for (auto &e : edges)
        Edges.erase(std::remove(Edges.begin(), Edges.end(), e), Edges.end());
    for (auto &v : orphanVertices)
        Vertices.erase(std::remove(Vertices.begin(), Vertices.end(), v), Vertices.end());
    fmt::println("Faces: {}, Edges: {}, Vertex: {}", Faces.size(), Edges.size(), Vertices.size());
}

void HalfEdgeMesh::Extrude(const HalfEdge::Face::Ptr &fromFace, float distance)
{
    // std::vector<HalfEdge::Ptr> newEdges;
    // std::vector<HalfEdge::Vertex::Ptr> newVertices;
    // std::vector<HalfEdge::Face::Ptr> newFaces;
    // std::unordered_map<HalfEdge::Vertex::Ptr, HalfEdge::Vertex::Ptr> vertexMap;

    // auto faceEdge = fromFace->Edge;
    // auto dir = fromFace->Normal() * distance;
    // DeleteFace(fromFace);

    // // vertices first
    // auto e = faceEdge.lock();
    // do
    // {
    //     auto extrudedVertex = HalfEdge::Vertex::New(e->Origin.lock()->P + dir);
    //     vertexMap[e->Origin.lock()] = extrudedVertex;
    //     newVertices.push_back(extrudedVertex);
    //     e = e->Next.lock();
    // } while (e != faceEdge.lock());

    // // connect half-edges to new vertices
    // e = faceEdge;
    // do
    // {
    //     auto edge = HalfEdge::New();
    //     // auto twin = HalfEdge::New();
    //     auto face = HalfEdge::Face::New();

    //     newEdges.push_back(edge);
    //     newFaces.push_back(face);

    //     face->Edge = edge;

    //     edge->Origin = e->Origin;
    //     edge->IncidentFace = face;
    //     edge->Next = nullptr; // later
    //     edge->Prev = nullptr; // later

    //     e = e->Next;
    // } while (e != faceEdge);

    // // build opposite face (extruded face along direction)
    // e = faceEdge;
    // do
    // {
    //     auto v = vertexMap[e->Origin];
    //     auto nextV = vertexMap[e->Next->Origin];
    //     auto edge = HalfEdge::New();
    //     auto twin = HalfEdge::New();

    //     e = e->Next;
    // } while (e != faceEdge);

    // for (auto &v : newVertices)
    //     Vertices.emplace_back(v);

    // for (auto &f : newFaces)
    //     Faces.emplace_back(f);

    // for (auto &e : newEdges)
    //     Edges.emplace_back(e);
}

Mesh HalfEdgeMesh::GenerateMesh(bool shareVertices) const
{
    Mesh mesh;
    if (Vertices.empty())
        return std::move(mesh);

    if (shareVertices)
    {
        std::map<HalfEdge::Vertex::Ptr, int> vertexMap;
        for (const auto &v : Vertices)
        {
            vertexMap[v] = mesh.Vertices.size();
            mesh.Vertices.emplace_back(v->P);
            // fmt::println("Vertex: {}, {}, {}", v->P.x, v->P.y, v->P.z);
            mesh.Colors.push_back(YELLOW);
            auto normal = v->IncidentEdge.lock()->IncidentFace.lock()->Normal();
            mesh.Normals.emplace_back(normal);
        }

        for (const auto &f : Faces)
        {
            auto eachTriangle = [&](const HalfEdge::Vertex::Ptr &a, const HalfEdge::Vertex::Ptr &b, const HalfEdge::Vertex::Ptr &c)
            {
                mesh.Indices.push_back(vertexMap[a]);
                mesh.Indices.push_back(vertexMap[b]);
                mesh.Indices.push_back(vertexMap[c]);
                auto normals = a->IncidentEdge.lock()->IncidentFace.lock()->Normal() + b->IncidentEdge.lock()->IncidentFace.lock()->Normal() + c->IncidentEdge.lock()->IncidentFace.lock()->Normal();
                normals = Mathf::Normalize(normals / 3.0f);
                mesh.Normals[vertexMap[a]] = normals;
                mesh.Normals[vertexMap[b]] = normals;
                mesh.Normals[vertexMap[c]] = normals;
                // fmt::println("Triangle: {}, {}, {}", vertexMap[a], vertexMap[b], vertexMap[c]);
            };
            f->EachTriangle(eachTriangle);
        }
    }
    else
    {
        // TODO: this is temporary
        std::vector<Vec4> colors = {
            Vec4(0.0f, 0.0f, 0.0f, 1.0f),
            Vec4(1.0f, 0.0f, 0.0f, 1.0f),
            Vec4(0.0f, 1.0f, 0.0f, 1.0f),
            Vec4(0.0f, 0.0f, 1.0f, 1.0f),
            Vec4(1.0f, 1.0f, 0.0f, 1.0f),
            Vec4(1.0f, 0.0f, 1.0f, 1.0f),
            Vec4(0.0f, 1.0f, 1.0f, 1.0f),
            Vec4(1.0f, 1.0f, 1.0f, 1.0f),
            Vec4(0.5f, 0.0f, 0.0f, 1.0f),
            Vec4(0.0f, 0.5f, 0.0f, 1.0f),
            Vec4(0.0f, 0.0f, 0.5f, 1.0f),
            Vec4(0.5f, 0.5f, 0.0f, 1.0f),
            Vec4(0.5f, 0.0f, 0.5f, 1.0f),
            Vec4(0.0f, 0.5f, 0.5f, 1.0f),
            Vec4(0.5f, 0.5f, 0.5f, 1.0f),
            Vec4(0.25f, 0.0f, 0.0f, 1.0f),
            Vec4(0.0f, 0.25f, 0.0f, 1.0f),
            Vec4(0.0f, 0.0f, 0.25f, 1.0f),
            Vec4(0.25f, 0.25f, 0.0f, 1.0f),
            Vec4(0.25f, 0.0f, 0.25f, 1.0f),
            Vec4(0.0f, 0.25f, 0.25f, 1.0f),
            Vec4(0.25f, 0.25f, 0.25f, 1.0f),
            Vec4(0.75f, 0.0f, 0.0f, 1.0f),
            Vec4(0.0f, 0.75f, 0.0f, 1.0f),
            Vec4(0.0f, 0.0f, 0.75f, 1.0f),
            Vec4(0.75f, 0.75f, 0.0f, 1.0f),
            Vec4(0.75f, 0.0f, 0.75f, 1.0f),
            Vec4(0.0f, 0.75f, 0.75f, 1.0f),
            Vec4(0.75f, 0.75f, 0.75f, 1.0f),
            Vec4(0.25f, 0.0f, 0.0f, 1.0f),
            Vec4(0.0f, 0.25f, 0.0f, 1.0f),
            Vec4(0.0f, 0.0f, 0.25f, 1.0f),
            Vec4(0.25f, 0.25f, 0.0f, 1.0f),
            Vec4(0.25f, 0.0f, 0.25f, 1.0f),
            Vec4(0.0f, 0.25f, 0.25f, 1.0f),
            Vec4(0.25f, 0.25f, 0.25f, 1.0f),
        };
        int colorIndex = 0;

        for (const auto &f : Faces)
        {
            std::map<HalfEdge::Vertex::Ptr, int> vertexMap;
            auto normal = f->Normal();
            auto color = colors[colorIndex % colors.size()];
            colorIndex++;
            auto eachTriangle = [&](const HalfEdge::Vertex::Ptr &a, const HalfEdge::Vertex::Ptr &b, const HalfEdge::Vertex::Ptr &c)
            {
                auto addVertex = [&](const HalfEdge::Vertex::Ptr &v)
                {
                    auto it = vertexMap.find(v);
                    if (it == vertexMap.end())
                    {
                        vertexMap[v] = mesh.Vertices.size();
                        mesh.Vertices.emplace_back(v->P);
                        mesh.Colors.push_back(color);
                        mesh.Normals.push_back(normal);
                    }
                };
                addVertex(a);
                addVertex(b);
                addVertex(c);
                mesh.Indices.push_back(vertexMap[a]);
                mesh.Indices.push_back(vertexMap[b]);
                mesh.Indices.push_back(vertexMap[c]);
            };

            f->EachTriangle(eachTriangle);
        }
    }

    return std::move(mesh);
}

void HalfEdgeMesh::DebugDrawLine(const std::function<void(const DrawLine &)> &fn, const Vec3 &cameraPosition) const
{
    DrawLine draw;
    for (const auto &e : Edges)
    {
        draw.Boundary = e->IncidentFace.expired();
        if (draw.Boundary)
        {
            draw.From = e->Origin.lock()->P;
            draw.To = e->Next.lock()->Origin.lock()->P;
            draw.Boundary = true;
            draw.Visible = true; // would be nice to have visibility check (twin face check)
            fn(draw);
        }
        else
        {
            auto center = e->IncidentFace.lock()->Center();
            auto dir = Mathf::Normalize(cameraPosition - center);
            auto dot = Mathf::Dot(dir, e->IncidentFace.lock()->Normal());
            draw.From = Mathf::Normalize(center - e->Origin.lock()->P) * 0.01f + e->Origin.lock()->P;
            draw.To = Mathf::Normalize(center - e->Next.lock()->Origin.lock()->P) * 0.01f + e->Next.lock()->Origin.lock()->P;
            draw.Visible = dot >= 0;
            fn(draw);
        }
    }
}

void HalfEdgeMesh::DebugDrawPoint(const std::function<void(const DrawPoint &)> &fn, const Vec3 &cameraPosition) const
{
    DrawPoint draw;
    for (const auto &v : Vertices)
    {
        draw.Position = v->P;
        draw.Center = false;
        draw.Visible = true; // would be nice to have visibility check (face check)
        fn(draw);
    }

    for (const auto &f : Faces)
    {
        auto center = f->Center();
        auto dir = Mathf::Normalize(cameraPosition - center);
        auto dot = Mathf::Dot(dir, f->Normal());

        draw.Position = center;
        draw.Center = true;
        draw.Visible = dot >= 0;
        fn(draw);
    }
}

void HalfEdgeMesh::DebugDrawNormal(const std::function<void(const DrawNormal &)> &fn, const Vec3 &cameraPosition) const
{
    DrawNormal draw;
    for (const auto &f : Faces)
    {
        draw.From = f->Center();
        draw.Direction = f->Normal();
        auto dir = Mathf::Normalize(cameraPosition - draw.From);
        auto dot = Mathf::Dot(dir, draw.Direction);
        draw.Visible = dot >= 0;
        fn(draw);
    }
}

HalfEdgeMesh::RaycastHit HalfEdgeMesh::Raycast(const Ray &ray) const
{
    RaycastHit hit;
    float minDistance = 100;
    for (const auto &f : Faces)
    {
        auto e = f->Edge.lock();
        do
        {
            auto a = e->Origin.lock()->P;
            auto b = e->Next.lock()->Origin.lock()->P;
            auto c = e->Next.lock()->Next.lock()->Origin.lock()->P;
            auto normal = -Mathf::Normalize(Mathf::Cross(b - a, c - a));
            if (Mathf::Dot(ray.Direction, normal) < 0)
                continue;
            auto intersection = Mathf::RayTriangleIntersection(ray, a, b, c, minDistance);

            // if (d > 0 && d < minDistance)
            if (intersection)
            {
                // minDistance = d;
                fmt::println("Intersection: {}", intersection);
                fmt::println("Distance: {}", minDistance);
                hit.Center = f->Center();
                hit.Face = f;
                break;
            }
            e = e->Next.lock();
        } while (e != f->Edge.lock());
    }
    return hit;
}

void HalfEdgeMesh::generateMissingTwins()
{
    std::map<std::pair<HalfEdge::Vertex::Ptr, HalfEdge::Vertex::Ptr>, HalfEdge::Ptr> edgeMap;
    for (const auto &e : Edges)
    {
        auto key = std::make_pair(e->Origin.lock(), e->Next.lock()->Origin.lock());
        edgeMap[key] = e;
    }

    for (const auto &e : Edges)
    {
        auto key = std::make_pair(e->Next.lock()->Origin.lock(), e->Origin.lock());
        if (edgeMap.find(key) != edgeMap.end())
        {
            auto twin = edgeMap[key];
            e->Twin = twin;
            twin->Twin = e;
        }
    }
}

bool HalfEdgeMeshSelection::IsSelected() const
{
    for (const auto &f : SelectedFaces)
    {
        if (!f.expired())
            return true;
    }
    return false;
}

void HalfEdgeMeshSelection::Select(const HalfEdge::Face::Ref &face)
{
    Clear();
    SelectedFaces.emplace_back(face);
}

void HalfEdgeMeshSelection::Clear()
{
    SelectedFaces.clear();
}

void HalfEdgeMeshSelection::DrawLine(const std::function<void(const HalfEdgeMesh::DrawLine &)> &fn, const Vec3 &cameraPosition) const
{
    HalfEdgeMesh::DrawLine draw;

    for (const auto &f : SelectedFaces)
    {
        if (f.expired())
            continue;

        auto center = f.lock()->Center();
        auto dir = Mathf::Normalize(cameraPosition - center);
        auto dot = Mathf::Dot(dir, f.lock()->Normal());

        auto e = f.lock()->Edge.lock();
        do
        {
            draw.From = e->Origin.lock()->P;
            draw.To = e->Next.lock()->Origin.lock()->P;
            draw.Boundary = e->IncidentFace.expired();
            draw.Visible = dot >= 0;
            fn(draw);
            e = e->Next.lock();
        } while (e != f.lock()->Edge.lock());
    }
}
