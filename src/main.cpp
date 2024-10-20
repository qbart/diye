#include <fmt/core.h>

#include "glfw.hpp"
#include "input.hpp"
#include "window.hpp"
#include "geometry.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "ui.hpp"
#include "image.hpp"

std::vector<Vec2> TileQuadUV(float TileX, float TileY)
{

    float tileWidth = 16.0f / 272.0f;  // normalized width of one tile
    float tileHeight = 16.0f / 288.0f; // normalized height of one tile

    float u1 = TileX * tileWidth;  // left
    float v1 = TileY * tileHeight; // top
    float u2 = u1 + tileWidth;     // right
    float v2 = v1 + tileHeight;    // bottom
    std::vector<Vec2> uvCoords = {
        {u1, v2},  // Bottom-left
        {u2, v2}, // Bottom-right
        {u1, v1}, // Top-left
        {u2, v2}, // Bottom-right
        {u2, v1}, // Top-right
        {u1, v1}, // Top-left
    };
    return uvCoords;
}

int main()
{
    auto window = Window::New(1600, 1000, "app");
    if (window == nullptr)
    {
        fmt::print("Failed to create window\n");
        return -1;
    }

    UI ui(window->Get());

    Input input = window->GetInput();
    window->Debug();

    auto cube = std::move(CubeMesh(1.0f));
    auto quad = std::move(QuadMesh(1.0f));
    quad.UVs = TileQuadUV(0,0);
    Transform transform;
    transform.position = Vec3(0.0f, 0.0f, 0.0f);
    transform.Update();

    Camera camera;
    camera.SetPosition(Vec3(0.0f, 5.0f, 5.0f));
    camera.LookAt(Vec3(0.0f, 0.0f, 0.0f));

    GL gl;
    gl.Defaults();
    auto vao = gl.CreateVertexArray();
    auto vbo = gl.GenBuffer();
    auto uvbo = gl.GenBuffer();
    auto ibo = gl.GenBuffer();
    auto cbo = gl.GenBuffer();

    {
        gl.BindVertexArray(vao);

        gl.EnableVertexAttribArray(0);
        gl.BindBuffer(GL::BufferType::Array, vbo);
        gl.BufferData(GL::BufferType::Array, quad.Vertices, GL::BufferUsage::Stream);
        gl.VertexAttribPointer(0, 3);

        gl.EnableVertexAttribArray(1);
        gl.BindBuffer(GL::BufferType::Array, cbo);
        gl.BufferData(GL::BufferType::Array, quad.Colors, GL::BufferUsage::Stream);
        gl.VertexAttribPointer(1, 3);

        gl.EnableVertexAttribArray(2);
        gl.BindBuffer(GL::BufferType::Array, uvbo);
        gl.BufferData(GL::BufferType::Array, quad.UVs, GL::BufferUsage::Stream);
        gl.VertexAttribPointer(2, 2);

        gl.BindBuffer(GL::BufferType::ElementArray, ibo);
        gl.BufferData(GL::BufferType::ElementArray, quad.Indices, GL::BufferUsage::Stream);

        gl.BindVertexArray(0);
        gl.BindBuffer(GL::BufferType::Array, 0);
        gl.BindBuffer(GL::BufferType::ElementArray, 0);
    }

    auto program = gl.CreateDefaultProgram(VertexShaderSource, FragmentShaderSource);
    if (program == 0)
    {
        fmt::print("Failed to create program\n");
        return -1;
    }
    auto modelLoc = gl.GetUniformLocation(program, "model");
    auto viewLoc = gl.GetUniformLocation(program, "view");
    auto projLoc = gl.GetUniformLocation(program, "projection");

    double lastTime = glfwGetTime();
    float dt = 0;

    fmt::println("Loading textures");
    Image tileset;
    if (!tileset.Load("./res/debug.png"))
    {
        fmt::print("Failed to load image {}\n", "res/cliffs_city_tileset.png");
        return -1;
    }
    fmt::println("Uploading textures to GPU");
    auto tilesetID = gl.GenTexture();
    gl.BindTexture(GL::TextureType::Texture2D, tilesetID);
    gl.TextureParameter(GL::TextureType::Texture2D, GL::TextureParameterName::WrapS, GL::TextureParameter::ClampToEdge);
    gl.TextureParameter(GL::TextureType::Texture2D, GL::TextureParameterName::WrapT, GL::TextureParameter::ClampToEdge);
    gl.TextureParameter(GL::TextureType::Texture2D, GL::TextureParameterName::MinFilter, GL::TextureParameter::Nearest);
    gl.TextureParameter(GL::TextureType::Texture2D, GL::TextureParameterName::MagFilter, GL::TextureParameter::Nearest);
    gl.TextureImage2D(GL::TextureType::Texture2D, tileset);
    gl.BindTexture(GL::TextureType::Texture2D, 0);

    fmt::println("Entering main loop");
    while (window->IsOpen())
    {
        // inputs
        window->PollEvents();
        if (input.KeyReleasedOnce(GLFW_KEY_ESCAPE))
            window->Close();

        // render
        auto size = window->Size();
        gl.Viewport(size.w, size.h);
        gl.ClearDepthBuffer();
        gl.ColorColorBuffer(Vec3(0.3f, 0.3f, 0.3f));
        camera.UpdatePerspective(size);

        transform.rotation = glm::rotate(transform.rotation, glm::radians(3.0f) * dt, UP);
        transform.Update();

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
        auto mat = transform.GetModelMatrix();
        auto view = camera.GetViewMatrix();
        auto proj = camera.GetProjection();
        gl.UseProgram(program);
        gl.ActiveTexture(GL::TextureUnit::Texture0);
        gl.BindTexture(GL::TextureType::Texture2D, tilesetID);
        gl.Uniform(modelLoc, mat);
        gl.Uniform(viewLoc, view);
        gl.Uniform(projLoc, proj);
        gl.BindVertexArray(vao);
        // gl.BindBuffer(GL::BufferType::Array, vbo);
        // gl.BufferData(GL::BufferType::Array, cube.Vertices, GL::BufferUsage::Stream);
        // gl.BindBuffer(GL::BufferType::Array, color);
        // gl.BufferData(GL::BufferType::Array, cube.Colors, GL::BufferUsage::Stream);
        // gl.BindBuffer(GL::BufferType::ElementArray, ibo);
        // gl.BufferData(GL::BufferType::ElementArray, cube.Indices, GL::BufferUsage::Stream);
        // gl.DrawElements(GL::DrawMode::Triangles, cube.Indices.size());
        gl.BindBuffer(GL::BufferType::Array, vbo);
        gl.BufferData(GL::BufferType::Array, quad.Vertices, GL::BufferUsage::Stream);
        gl.BindBuffer(GL::BufferType::Array, cbo);
        gl.BufferData(GL::BufferType::Array, quad.Colors, GL::BufferUsage::Stream);
        gl.BindBuffer(GL::BufferType::Array, uvbo);
        gl.BufferData(GL::BufferType::Array, quad.UVs, GL::BufferUsage::Stream);
        gl.BindBuffer(GL::BufferType::ElementArray, ibo);
        gl.BufferData(GL::BufferType::ElementArray, quad.Indices, GL::BufferUsage::Stream);
        gl.DrawElements(GL::DrawMode::Triangles, quad.Indices.size());
        gl.BindTexture(GL::TextureType::Texture2D, 0);
        gl.BindBuffer(GL::BufferType::Array, 0);
        gl.BindBuffer(GL::BufferType::ElementArray, 0);
        gl.BindVertexArray(0);

        // ui
        ui.BeginFrame();
        // ui.Demo();
        ui.EndFrame();
        ui.Draw();

        // swap
        window->Swap();
        dt = glfwGetTime() - lastTime;
        lastTime = glfwGetTime();
    }

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

    return 0;
}