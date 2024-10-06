#include <fmt/core.h>

#include "glfw.hpp"
#include "input.hpp"
#include "window.hpp"
#include "geometry.hpp"

int main()
{
    auto window = Window::New(1024, 768, "app");

    Input input = window->GetInput();
    window->Debug();

    GL gl;
    gl.CullFace(true, GL::Face::Back);
    gl.FrontFace(GL::FrontFace::CounterClockwise);

    auto cube = std::move(CubeMesh(1.0f));

    while (window->IsOpen())
    {
		auto size = window->Size();
        auto proj = Mathf::Fov(55, size.w, size.h);
		auto model = Mat4(1.0f);
        gl.Viewport(size.w, size.h);
        gl.ClearDepthBuffer();
        gl.ColorColorBuffer(Vec3(0.3f, 0.3f, 0.3f));

        window->PollEvents();
        window->Swap();

        if (input.KeyReleasedOnce(GLFW_KEY_ESCAPE))
            window->Close();
    }

    return 0;
}