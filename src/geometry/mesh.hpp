#pragma once

#include "../core/all.hpp"
#include <memory>

class Mesh
{
public:
    using Ptr = std::unique_ptr<Mesh>;

    std::vector<Vec3> Vertices;
    std::vector<Vec3> Colors;
    std::vector<Vec3> Normals;
    std::vector<uint32> Indices;
};