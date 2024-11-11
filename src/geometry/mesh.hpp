#pragma once

#include "../core/all.hpp"

struct Mesh
{
public:
    std::vector<Vec3> Vertices;
    std::vector<Vec3> Colors;
    std::vector<uint32> Indices;
};