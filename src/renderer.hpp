#pragma once

#include "core/all.hpp"
#include "gl.hpp"
#include "geometry.hpp"
#include "shader.hpp"
#include "image.hpp"

std::vector<Vec2> TileQuadUV(float TileX, float TileY)
{

    float tileWidth = 16.0f / 272.0f;  // normalized width of one tile
    float tileHeight = 16.0f / 288.0f; // normalized height of one tile

    // float u2 = TileX * tileWidth;  // left
    // float v2 = TileY * tileHeight; // top
    // float u1 = u2 + tileWidth;     // right
    // float v1 = v2 + tileHeight;    // bottom
    // std::vector<Vec2> uvCoords = {
    //     {u1, v2},  // Bottom-left
    //     {u2, v2}, // Bottom-right
    //     {u1, v1}, // Top-left
    //     {u2, v2}, // Bottom-right
    //     {u2, v1}, // Top-right
    //     {u1, v1}, // Top-left
    // };

    float u1 = TileX * tileWidth;               // left
    float u2 = u1 + tileWidth;                  // right
    float v1 = TileY * tileHeight + tileHeight; // top
    float v2 = v1 - tileHeight;                 // bottom
    std::vector<Vec2> uvCoords = {
        {u1, v2}, // Bottom-left
        {u2, v2}, // Bottom-right
        {u1, v1}, // Top-left
        {u2, v2}, // Bottom-right
        {u2, v1}, // Top-right
        {u1, v1}, // Top-left
    };
    return uvCoords;
}

class Renderer
{
public:
    uint vao;
    GL::Buffer vbo;
    GL::Buffer uvbo;
    GL::Buffer ibo;
    GL::Buffer cbo;
    GL::Program program;
    GL gl;
    GL::Texture tilesetID;
    GL::Location modelLoc;
    GL::Location viewLoc;
    GL::Location projLoc;
    Geometry mesh;

public:
    int Init()
    {
        mesh = std::move(TiledMesh(3, 3));
        vao = gl.CreateVertexArray();
        vbo = gl.GenBuffer();
        uvbo = gl.GenBuffer();
        ibo = gl.GenBuffer();
        cbo = gl.GenBuffer();

        {
            gl.BindVertexArray(vao);

            gl.EnableVertexAttribArray(0);
            gl.BindBuffer(GL::BufferType::Array, vbo);
            gl.BufferData(GL::BufferType::Array, mesh.Vertices, GL::BufferUsage::Stream);
            gl.VertexAttribPointer(0, 3);

            gl.EnableVertexAttribArray(1);
            gl.BindBuffer(GL::BufferType::Array, cbo);
            gl.BufferData(GL::BufferType::Array, mesh.Colors, GL::BufferUsage::Stream);
            gl.VertexAttribPointer(1, 3);

            gl.EnableVertexAttribArray(2);
            gl.BindBuffer(GL::BufferType::Array, uvbo);
            gl.BufferData(GL::BufferType::Array, mesh.UVs, GL::BufferUsage::Stream);
            gl.VertexAttribPointer(2, 2);

            gl.BindBuffer(GL::BufferType::ElementArray, ibo);
            gl.BufferData(GL::BufferType::ElementArray, mesh.Indices, GL::BufferUsage::Stream);

            gl.BindVertexArray(0);
            gl.BindBuffer(GL::BufferType::Array, 0);
            gl.BindBuffer(GL::BufferType::ElementArray, 0);
        }

        program = gl.CreateDefaultProgram(VertexShaderSource, FragmentShaderSource);
        if (program == 0)
        {
            fmt::print("Failed to create program\n");
            return -1;
        }
        modelLoc = gl.GetUniformLocation(program, "model");
        viewLoc = gl.GetUniformLocation(program, "view");
        projLoc = gl.GetUniformLocation(program, "projection");

        fmt::println("Loading textures");
        Image tileset;
        if (!tileset.Load("../res/debug.png"))
        {
            fmt::print("Failed to load image {}\n", "res/cliffs_city_tileset.png");
            return -1;
        }
        fmt::println("Uploading textures to GPU");
        tilesetID = gl.CreateDefaultTexture(tileset);

        return 0;
    }

    void Render(const Camera &camera)
    {
        // for each ( render target )			// frame buffer
        // for each ( pass )					// depth, blending, etc. states
        // for each ( material )				// shaders
        // for each ( material instance )		// textures
        // for each ( vertex format )			// vertex buffers
        // for each ( object )
        // {
        //		WriteUniformData(object);
        //		glDrawElementsBaseVertex(
        //			GL_TRIANGLES,
        //			object->indexCount,
        //			GL_UNSIGNED_SHORT,
        //			object->indexDataOffset,
        //			object->baseVertex
        //		);
        // }
        auto view = camera.View();
        auto proj = camera.Projection();
        auto mat = Mat4(1);
        gl.UseProgram(program);
        gl.ActiveTexture(GL::TextureUnit::Texture0);
        gl.BindTexture(GL::TextureType::Texture2D, tilesetID);
        gl.Uniform(modelLoc, mat);
        gl.Uniform(viewLoc, view);
        gl.Uniform(projLoc, proj);
        gl.BindVertexArray(vao);
        gl.BindBuffer(GL::BufferType::Array, vbo);
        gl.BufferData(GL::BufferType::Array, mesh.Vertices, GL::BufferUsage::Stream);
        gl.BindBuffer(GL::BufferType::Array, cbo);
        gl.BufferData(GL::BufferType::Array, mesh.Colors, GL::BufferUsage::Stream);
        gl.BindBuffer(GL::BufferType::Array, uvbo);
        gl.BufferData(GL::BufferType::Array, mesh.UVs, GL::BufferUsage::Stream);
        gl.BindBuffer(GL::BufferType::ElementArray, ibo);
        gl.BufferData(GL::BufferType::ElementArray, mesh.Indices, GL::BufferUsage::Stream);
        gl.DrawElements(GL::DrawMode::Triangles, mesh.Indices.size());
        gl.BindTexture(GL::TextureType::Texture2D, 0);
        gl.BindBuffer(GL::BufferType::Array, 0);
        gl.BindBuffer(GL::BufferType::ElementArray, 0);
        gl.BindVertexArray(0);
    }

    void Shutdown()
    {
        gl.BindTexture(GL::TextureType::Texture2D, 0);
        gl.BindBuffer(GL::BufferType::Array, 0);
        gl.BindBuffer(GL::BufferType::ElementArray, 0);
        gl.BindVertexArray(0);
        gl.DeleteTexture(tilesetID);
        gl.DeleteBuffer(vbo);
        gl.DeleteBuffer(ibo);
        gl.DeleteBuffer(cbo);
        gl.DeleteVertexArray(vao);
        gl.DeleteProgram(program);
    }
};