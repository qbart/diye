#pragma once

#include "../core/all.hpp"

class HalfEdge
{
public:
    enum class InitType
    {
        NONE,
        PLANE,
        CUBE,
    };
    struct Face
    {
        HalfEdge *Edge;
    };
    struct Vertex
    {
        Vec3 P;
        HalfEdge *IncidentEdge;
    };

public:
    HalfEdge(InitType type = InitType::NONE);
    ~HalfEdge();

private:
    Vertex *origin;
    HalfEdge *twin;
    Face *incidentFace;
    HalfEdge *prev;
    HalfEdge *next;
};