#include <fmt/core.h>

#include "window.hpp"
#include "ui/ui.hpp"
#include "debug_draw_renderer.hpp"
#include "experiments/experiment.hpp"
#include "gl/app.hpp"

int main()
{
    sdl::Window window;
    gl::App app;
    if (!window.Init(1600, 1000, "app"))
    {
        fmt::print("Failed to create window\n");
        return 1;
    }
    if (!app.Init(window.Get()))
    {
        fmt::print("Failed to init app\n");
        return 1;
    }
    // UI ui(window->Get());

    Camera camera;
    camera.SetPosition(Vec3(0.0f, 5.0f, 5.0f));
    // camera.LookAt(ZERO);
    camera.SetPosition(Vec3(2.0f, 2.0f, 2.0f));
    camera.LookAt(ZERO);

    Transform transform;

    // fmt::println("Initializing debug renderer");
    // DebugDrawRenderer debug;

    sdl::Ticks ticks;
    float dt = 0;

    fmt::println("Entering main loop");
    auto experiment = std::make_unique<EmptyExperiment>();
    if (experiment->Init(&window) != 0)
    {
        fmt::print("Failed to init experiment\n");
        return -1;
    }

    while (window.IsOpen())
    {
        // ---------- inputs -----------
        window.PollEvents();
        app.RequestRecreateSwapChain(window.WasResized());

        if (window.KeyJustReleased(SDLK_ESCAPE))
            window.Close();

        if (window.KeyDown(SDLK_w))
            camera.MoveForward(5 * dt);

        if (window.KeyDown(SDLK_s))
            camera.MoveBackward(5 * dt);

        if (window.KeyDown(SDLK_a))
            camera.MoveLeft(5 * dt);

        if (window.KeyDown(SDLK_d))
            camera.MoveRight(5 * dt);

        if (window.MouseButtonDown(SDL_BUTTON_RIGHT) && window.KeyDown(SDLK_LALT))
        {
            auto md = window.MouseRelativePosition();
            camera.OrbitAround(UP, ZERO, md.x * 2 * dt);
            camera.OrbitAround(LEFT, ZERO, -md.y * 2 * dt);
        }
        else if (window.MouseButtonDown(SDL_BUTTON_RIGHT))
        {
            auto md = window.MouseRelativePosition();
            camera.LookAround(md.y * 10 * dt, md.x * 10 * dt);
        }
        if (window.MouseWheelScrolled())
        {
            auto mw = window.MouseWheel();
            camera.MoveForward(window.MouseWheel().y * dt * 5);
        }

        // ---------- update -----------
        ticks.Update();
        dt = ticks.DeltaTime();
        auto size = window.Size();
        camera.UpdatePerspective(size);
        // experiment->Update(dt);

        // ---------- render -----------
        // gl.Viewport(size.w, size.h);
        // gl.ClearDepthBuffer();
        // gl.ClearColorBuffer(Vec3(0.3f, 0.3f, 0.3f));
        // experiment->Render(camera);

        // ---------- render:debug -----------
        // debug.Begin(size, camera);
        // debug.Grid(-5.0f, 5.0f, -0.005f, 1.f, GRAY);
        // debug.Grid(-5.0f, 5.0f, -0.005f, 0.25f, BLACK);
        // experiment->RenderDebug(camera, debug);
        // debug.End();

        // ---------- render:ui -----------
        // ui.BeginFrame(size);
        // experiment->RenderUI(camera, ui);
        // ui.EndFrame();
        // ui.Draw();
        if (!app.BeginFrame())
            window.Close();

        // transform.rotation = Mathf::Rotate(Mat4(1), 90 * time, UP);
        auto mvp = camera.MVP(transform.ModelMatrix());
        if (!app.Render(mvp))
            window.Close();

        if (!app.EndFrame())
            window.Close();
    }
    app.Shutdown();

    return 0;
}