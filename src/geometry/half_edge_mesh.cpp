#include "half_edge_mesh.hpp"

HalfEdgeMesh::Ptr HalfEdgeMesh::New(Preset preset)
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
    // if (!Origin)
    //     return std::move(mesh);

    // // Collect vertices and build index map
    // std::map<Vertex::Ptr, size_t> vertexToIndex;
    // std::set<Face::Ptr> processedFaces;
    // std::queue<HalfEdge *> queue;
    // queue.push(const_cast<HalfEdge *>(this));

    // // First pass: collect vertices
    // while (!queue.empty())
    // {
    //     HalfEdge *current = queue.front();
    //     queue.pop();

    //     // Add vertex if not already added
    //     if (vertexToIndex.find(current->Origin) == vertexToIndex.end())
    //     {
    //         vertexToIndex(current->Origin) = mesh.Vertices.size();
    //         mesh.Vertices.push_back(current->Origin->P);

    //         // Cycle through red, green, blue for vertex colors
    //         switch (mesh.Vertices.size() % 3)
    //         {
    //         case 0:
    //             mesh.Colors.push_back({0.0f, 0.0f, 1.0f});
    //             break; // Blue
    //         case 1:
    //             mesh.Colors.push_back({1.0f, 0.0f, 0.0f});
    //             break; // Red
    //         case 2:
    //             mesh.Colors.push_back({0.0f, 1.0f, 0.0f});
    //             break; // Green
    //         }
    //     }

    //     // Add face to process if not already processed
    //     if (current->incidentFace && processedFaces.find(current->incidentFace) == processedFaces.end())
    //     {
    //         processedFaces.insert(current->incidentFace);

    //         // Generate triangular face indices
    //         HalfEdge *edge = current;
    //         Vertex *v0 = edge->Origin;
    //         edge = edge->next;
    //         Vertex *v1 = edge->Origin;
    //         edge = edge->next;

    //         while (edge != current)
    //         {
    //             Vertex *v2 = edge->Origin;
    //             // Add triangle indices
    //             mesh.Indices.push_back(vertexToIndex[v0]);
    //             mesh.Indices.push_back(vertexToIndex[v1]);
    //             mesh.Indices.push_back(vertexToIndex[v2]);

    //             v1 = v2;
    //             edge = edge->next;
    //         }
    //     }

    //     // Queue up connected edges if not processed
    //     if (current->twin && processedFaces.find(current->twin->incidentFace) == processedFaces.end())
    //     {
    //         queue.push(current->twin);
    //     }
    // }

    return std::move(mesh);
}