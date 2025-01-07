#pragma once

#include "../core/all.hpp"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_beta.h>
#include <tiny_obj_loader.h>

namespace io
{
    class OBJ
    {
    public:
        struct Vertex
        {
            Vec3 pos;
            Vec3 color;
            Vec2 texCoord;
        };

        struct Mesh
        {
            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;
        };

    public:
        OBJ();
        ~OBJ();

        bool Load(const std::string &filename);
        void Unload();
        const Mesh &GetMesh() const { return mesh; }

    private:
        void LoadMesh();

    private:
        Mesh mesh;
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
    };

}
