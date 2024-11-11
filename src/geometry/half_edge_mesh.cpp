#include "half_edge_mesh.hpp"
#include <map>

HalfEdgeMesh::Ptr HalfEdgeMesh::NewPlane()
{
    using Vertex = HalfEdge::Vertex;
    using Face = HalfEdge::Face;

    HalfEdge::Ptr e[10];
    Face::Ptr f[2];
    Vertex::Ptr v[4];

    v[0] = Vertex::New({0, 0, 1});
    v[1] = Vertex::New({1, 0, 1});
    v[2] = Vertex::New({0, 0, 0});
    v[3] = Vertex::New({1, 0, 0});

    for (int i = 0; i < 2; ++i)
        f[i] = Face::New();

    for (int i = 0; i < 10; ++i)
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

    // Face	Half-edge
    // f0	e0
    // f1	e3

    f[0]->Edge = e[0];
    f[1]->Edge = e[3];

    v[0]->IncidentEdge = e[0];
    v[1]->IncidentEdge = e[5];
    v[2]->IncidentEdge = e[1];
    v[3]->IncidentEdge = e[2];

    e[0]->Origin = v[0];
    e[0]->Twin = e[6];
    e[0]->IncidentFace = f[0];
    e[0]->Next = e[1];
    e[0]->Prev = e[2];

    e[1]->Origin = v[2];
    e[1]->Twin = e[7];
    e[1]->IncidentFace = f[0];
    e[1]->Next = e[2];
    e[1]->Prev = e[0];

    e[2]->Origin = v[3];
    e[2]->Twin = e[3];
    e[2]->IncidentFace = f[0];
    e[2]->Next = e[0];
    e[2]->Prev = e[1];

    e[3]->Origin = v[0];
    e[3]->Twin = e[2];
    e[3]->IncidentFace = f[1];
    e[3]->Next = e[4];
    e[3]->Prev = e[5];

    e[4]->Origin = v[3];
    e[4]->Twin = e[8];
    e[4]->IncidentFace = f[1];
    e[4]->Next = e[5];
    e[4]->Prev = e[3];

    e[5]->Origin = v[1];
    e[5]->Twin = e[9];
    e[5]->IncidentFace = f[1];
    e[5]->Next = e[3];
    e[5]->Prev = e[4];

    e[6]->Origin = v[2];
    e[6]->Twin = e[0];
    e[6]->IncidentFace = nullptr;
    e[6]->Next = e[9];
    e[6]->Prev = e[7];

    e[7]->Origin = v[3];
    e[7]->Twin = e[1];
    e[7]->IncidentFace = nullptr;
    e[7]->Next = e[6];
    e[7]->Prev = e[8];

    e[8]->Origin = v[1];
    e[8]->Twin = e[4];
    e[8]->IncidentFace = nullptr;
    e[8]->Next = e[7];
    e[8]->Prev = e[9];

    e[9]->Origin = v[0];
    e[9]->Twin = e[5];
    e[9]->IncidentFace = nullptr;
    e[9]->Next = e[8];
    e[9]->Prev = e[6];

    auto mesh = std::make_unique<HalfEdgeMesh>();
    for (int i = 0; i < 4; ++i)
        mesh->Vertices.emplace_back(v[i]);

    for (int i = 0; i < 2; ++i)
        mesh->Faces.emplace_back(f[i]);

    for (int i = 0; i < 10; ++i)
        mesh->Edges.emplace_back(e[i]);
    
    mesh->generateMissingTwins();

    return std::move(mesh);
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
    auto mesh = std::make_unique<HalfEdgeMesh>();
    for (int i = 0; i < 8; ++i)
        mesh->Vertices.emplace_back(v[i]);

    for (int i = 0; i < 6; ++i)
        mesh->Faces.emplace_back(f[i]);

    for (int i = 0; i < 24; ++i)
        mesh->Edges.emplace_back(e[i]);
    
    mesh->generateMissingTwins();

    return std::move(mesh);
}

void HalfEdgeMesh::EachVertex(const std::function<void(const HalfEdge::Vertex::Ptr &)> &fn) const
{
    for (const auto &v : Vertices)
        fn(v);
}

void HalfEdgeMesh::EachFace(const std::function<void(const HalfEdge::Face::Ptr &)> &fn) const
{
    for (const auto &f : Faces)
        fn(f);
}

void HalfEdgeMesh::EachHalfEdge(const std::function<void(const HalfEdge::Ptr &)> &fn) const
{
    for (const auto &e : Edges)
        fn(e);
}

Mesh &&HalfEdgeMesh::GenerateMesh() const
{
    Mesh mesh;
    if (Vertices.empty())
        return std::move(mesh);

    std::map<HalfEdge::Vertex::Ptr, int> vertexMap;
    for (const auto &v : Vertices)
    {
        vertexMap[v] = mesh.Vertices.size();
        mesh.Vertices.emplace_back(v->P);
    }

    for (const auto &f : Faces)
    {
        auto e = f->Edge;
        do
        {
            mesh.Indices.push_back(vertexMap[e->Origin]);
            e = e->Next;
        } while (e != f->Edge);
    }

    return std::move(mesh);
}

void HalfEdgeMesh::generateMissingTwins()
{
    std::map<std::pair<HalfEdge::Vertex::Ptr, HalfEdge::Vertex::Ptr>, HalfEdge::Ptr> edgeMap;
    for (const auto &e : Edges)
    {
        auto key = std::make_pair(e->Origin, e->Next->Origin);
        auto it = edgeMap.find(key);
        if (it != edgeMap.end())
        {
            e->Twin = it->second;
            it->second->Twin = e;
        }
        else
        {
            edgeMap[key] = e;
        }
    }    
}
