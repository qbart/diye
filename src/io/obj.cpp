#include "obj.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace io
{
    OBJ::OBJ()
    {
    }

    OBJ::~OBJ()
    {
        Unload();
    }

    bool OBJ::Load(const std::string &filename)
    {
        std::string warn, err;

        bool ok = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str());

        if (!warn.empty())
            fmtx::Warn(warn);
        if (!err.empty())
            fmtx::Error(err);

        if (ok)
            LoadMesh();

        return ok;
    }

    void OBJ::Unload()
    {
    }

    void OBJ::LoadMesh()
    {
        for (const auto &shape : shapes)
        {
            for (const auto &index : shape.mesh.indices)
            {
                Vertex vertex{};
                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]};

                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]}; // vulkan fix: 1 - v

                vertex.color = {1.0f, 1.0f, 1.0f};

                mesh.vertices.emplace_back(vertex);
                mesh.indices.emplace_back(mesh.indices.size());
            }
        }
    }
}