#include <fmt/core.h>

#include "glfw.hpp"
#include "input.hpp"
#include "window.hpp"
#include "geometry.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "ui.hpp"

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
    auto ibo = gl.GenBuffer();
    auto color = gl.GenBuffer();

    {
        gl.BindVertexArray(vao);

        gl.EnableVertexAttribArray(0);
        gl.BindBuffer(GL::BufferType::Array, vbo);
        gl.VertexAttribPointer(0, 3);
        // gl.BufferData(GL::BufferType::Array, cube.verts, GL::BufferUsage::Static);

        gl.EnableVertexAttribArray(1);
        gl.BindBuffer(GL::BufferType::Array, color);
        gl.VertexAttribPointer(1, 3);
        // gl.BufferData(GL::BufferType::Array, cube.colors, GL::BufferUsage::Static);

        gl.BindBuffer(GL::BufferType::ElementArray, ibo);
        // gl.BufferData(GL::BufferType::ElementArray, cube.indices, GL::BufferUsage::Static);

        gl.BindBuffer(GL::BufferType::Array, 0);
        gl.BindBuffer(GL::BufferType::ElementArray, 0);
        gl.BindVertexArray(0);
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
    fmt::print("model: {}, view: {}, proj: {}\n", modelLoc, viewLoc, projLoc);

    double lastTime = glfwGetTime();
    float dt = 0;

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

        transform.rotation = glm::rotate(transform.rotation, glm::radians(30.0f)*dt, UP);
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
        gl.UseProgram(program);
        auto mat = transform.GetModelMatrix();
        auto view = camera.GetViewMatrix();
        auto proj = camera.GetProjection();
        gl.Uniform(modelLoc, mat);
        gl.Uniform(viewLoc, view);
        gl.Uniform(projLoc, proj);
        gl.BindVertexArray(vao);
        gl.BindBuffer(GL::BufferType::Array, vbo);
        gl.BufferData(GL::BufferType::Array, cube.verts, GL::BufferUsage::Static);
        gl.BindBuffer(GL::BufferType::Array, color);
        gl.BufferData(GL::BufferType::Array, cube.colors, GL::BufferUsage::Static);
        gl.BindBuffer(GL::BufferType::ElementArray, ibo);
        gl.BufferData(GL::BufferType::ElementArray, cube.indices, GL::BufferUsage::Static);
        gl.DrawElements(GL::DrawMode::Triangles, cube.indices.size());

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

    gl.BindBuffer(GL::BufferType::Array, 0);
    gl.BindBuffer(GL::BufferType::ElementArray, 0);
    gl.BindVertexArray(0);
    gl.DeleteBuffer(vbo);
    gl.DeleteBuffer(ibo);
    gl.DeleteBuffer(color);
    gl.DeleteVertexArray(vao);
    gl.DeleteProgram(program);

    return 0;
}