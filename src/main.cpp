#include "deps/sdl.hpp"
#include "deps/fmt.hpp"
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
        fmtx::Error("Failed to create window");
        return 1;
    }
    fmtx::Success("Window initialized");

    if (!app.Init(window.Get()))
    {
        fmtx::Error("Failed to init Vulkan");
        return 1;
    }
    fmtx::Success("Vulkan initialized");
    // UI ui(window.Get());

    Camera camera;
    camera.SetPosition(Vec3(2.0f, 2.0f, 2.0f));
    camera.LookAt(ZERO);

    Transform transform;
    transform.Rotate(-90);

    // fmt::println("Initializing debug renderer");
    // DebugDrawRenderer debug;

    sdl::Ticks ticks;
    float dt = 0;

    fmtx::Debug("Entering main loop");
    auto experiment = std::make_unique<EmptyExperiment>();
    if (experiment->Init(&window) != 0)
    {
        fmtx::Error("Failed to init experiment");
        return -1;
    }

    while (window.IsOpen())
    {
        // ---------- inputs -----------
        window.PollEvents();
        window.FreeCameraControls(camera, dt);

        // ---------- update -----------
        ticks.Update();
        dt = ticks.DeltaTime();
        auto size = window.Size();
        camera.UpdatePerspective(size);
        // experiment->Update(dt);

        // ---------- render -----------
        app.RequestRecreateSwapChain(window.WasResized());
        if (!app.BeginFrame())
            window.Close();

        auto mvp = camera.MVP(transform.ModelMatrix());
        if (!app.Render(mvp))
            window.Close();

        if (!app.EndFrame())
            window.Close();
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

    }
    app.Shutdown();

    return 0;
}