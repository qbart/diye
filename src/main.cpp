#include <fmt/core.h>

#include "input.hpp"
#include "window.hpp"
#include "renderer.hpp"
#include "ui.hpp"
#include "debug_draw_renderer.hpp"
#include "experiment.hpp"

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

    Camera camera;
    camera.SetPosition(Vec3(0.0f, 5.0f, 5.0f));
    Vec3 orbitPoint(0, 2.5, 0);
    camera.LookAt(orbitPoint);

    GL gl;
    gl.Defaults();
    fmt::println("Initializing debug renderer");
    DebugDrawRenderer debug;

    SDLTicks ticks;
    float dt = 0;

    fmt::println("Entering main loop");
    auto experiment = std::make_unique<EmptyExperiment>();
    if (experiment->Init() != 0)
    {
        fmt::print("Failed to init experiment\n");
        return -1;
    }

    while (window->IsOpen())
    {
        // ---------- inputs -----------
        window->PollEvents();
        if (input.KeyJustReleased(SDLK_ESCAPE))
            window->Close();

        if (input.KeyDown(SDLK_w))
            camera.MoveForward(5 * dt);

        if (input.KeyDown(SDLK_s))
            camera.MoveBackward(5 * dt);

        if (input.KeyDown(SDLK_a))
            camera.MoveLeft(5 * dt);

        if (input.KeyDown(SDLK_d))
            camera.MoveRight(5 * dt);

        if (input.KeyDown(SDLK_q))
            camera.LookAround(0, -60 * dt);

        if (input.KeyDown(SDLK_e))
            camera.LookAround(0, 60 * dt);

        if (window->MouseButtonDown(SDL_BUTTON_RIGHT))
        {
            auto md = window->MouseRelativePosition();
            camera.Orbit(UP, orbitPoint, md.x * dt);
            camera.Orbit(LEFT, orbitPoint, -md.y * 2 * dt);
        }
        if (window->MouseWheelScrolled())
        {
            auto mw = window->MouseWheel();
            camera.MoveForward(window->MouseWheel().y * dt * 5);
        }

        // ---------- update -----------
        ticks.Update();
        dt = ticks.DeltaTime();
        auto size = window->Size();
        camera.UpdatePerspective(size);
        experiment->Update(dt);

        // ---------- render -----------
        gl.Viewport(size.w, size.h);
        gl.ClearDepthBuffer();
        gl.ColorColorBuffer(Vec3(0.3f, 0.3f, 0.3f));

        // ---------- render:debug -----------
        debug.Begin(size, camera);
        debug.Grid(-5.0f, 5.0f, -0.001f, 1.f, GRAY);
        debug.Grid(-5.0f, 5.0f, -0.001f, 0.25f, BLACK);
        experiment->Render(camera, debug);
        debug.End();

        // ---------- render:ui -----------
        ui.BeginFrame(size);
        experiment->RenderUI(ui);
        ui.EndFrame();
        ui.Draw();

        // ---------- swap chain -----------
        window->Swap();
    }

    return 0;
}