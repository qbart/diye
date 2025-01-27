#include "deps/sdl.hpp"
#include "deps/fmt.hpp"
#include "ui/ui.hpp"
#include "experiments/experiment.hpp"
#include "gl/app.hpp"
#include "gl/debug_renderer.hpp"

int main()
{
    sdl::Window window;
    gl::App app;
    UI ui;

    app.WithUI(true);

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

    if (!ui.Init(window.Get(), app))
    {
        fmtx::Error("Failed to init UI");
        return 1;
    }
    fmtx::Success("UI initialized");

    gl::DebugRenderer debug;
    if (!debug.Init(app.device, app.physicalDevice, app.renderPass))
    {
        fmtx::Error("Failed to init debug renderer");
        return 1;
    }
    fmtx::Success("Debug renderer initialized");

    Camera camera;
    camera.SetPosition(Vec3(2.0f, 2.0f, 2.0f));
    camera.LookAt(ZERO);

    Transform transform;
    transform.Rotate(-90);
    // transform.position = Vec3(0, 0.5f, 0);
    

    sdl::Ticks ticks;
    float dt = 0;

    fmtx::Debug("Entering main loop");
    auto experiment = std::make_unique<EmptyExperiment>();
    if (experiment->Init(&window) != 0)
    {
        fmtx::Error("Failed to init experiment");
        return -1;
    }

    transform.scale.x = 1;

    while (window.IsOpen())
    {
        // ---------- inputs -----------
        window.PollEvents(&ui);
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

        ui.BeginFrame(size);
        ui.ScaleGizmo(camera, transform);
        // ui.TranslateGizmo(camera, transform);
        // experiment->RenderUI(camera, ui);
        ui.EndFrame();

        app.commandBuffers.Reset(app.Frame());
        app.commandBuffers.Begin(app.Frame());
        app.commandBuffers.ClearColor({0.1f, 0.1f, 0.1f, 1.0f});
        app.commandBuffers.ClearDepthStencil();

        debug.Begin();
        debug.Grid(-5.0f, 5.0f, -0.005f, 0.25f, BLACK);
        debug.Grid(-5.0f, 5.0f, -0.005f, 1.f, GRAY);
        debug.Sphere(Vec3(1, 0, 0), 0.05f, RED);
        debug.Sphere(Vec3(0, 1, 0), 0.05f, GREEN);
        debug.Sphere(Vec3(0, 0, 1), 0.05f, BLUE);
        // debug.Cylinder(transform.position, UP, PURPLE, transform.scale.x, transform.scale.y);
        debug.Circle(transform.position, UP, transform.scale.x, YELLOW);
        // debug.Cone(transform.position, UP, MAGENTA, transform.scale.x, transform.scale.y);
        debug.End(app.commandBuffers.handles[app.Frame()]);

        app.commandBuffers.CmdBeginRenderPass(app.Frame(), app.renderPass, app.swapChainFramebuffers[app.ImageIndex()], app.swapChain.extent);
        {
            app.uniformBuffersMemory[app.Frame()].CopyRaw(app.device, &app.ubos[app.Frame()], sizeof(app.ubos[app.Frame()]));
            app.commandBuffers.CmdBindGraphicsPipeline(app.Frame(), app.graphicsPipeline);
            app.commandBuffers.CmdViewport(app.Frame(), {0, 0}, app.swapChain.extent);
            app.commandBuffers.CmdScissor(app.Frame(), {0, 0}, app.swapChain.extent);
            app.commandBuffers.CmdBindDescriptorSet(app.Frame(), app.graphicsPipeline, app.descriptorPool.descriptorSets[app.Frame()].handle);
            app.commandBuffers.CmdBindVertexBuffer(app.Frame(), app.vertexBuffer);
            app.commandBuffers.CmdBindIndexBuffer(app.Frame(), app.indexBuffer);
            // app.commandBuffers.CmdDrawIndexed(app.Frame(), static_cast<uint32_t>(app.indices.size()));

            debug.CmdDraw(camera, app.commandBuffers.handles[app.Frame()]);
            ui.CmdDraw(app.commandBuffers.handles[app.Frame()]);
        }
        app.commandBuffers.CmdEndRenderPass(app.Frame());
        app.commandBuffers.End(app.Frame());

        if (!app.EndFrame())
            window.Close();
        // experiment->Render(camera);

        // ---------- render:debug -----------
        // debug.Begin(size, camera);
        // debug.Grid(-5.0f, 5.0f, -0.005f, 1.f, GRAY);
        // debug.Grid(-5.0f, 5.0f, -0.005f, 0.25f, BLACK);
        // experiment->RenderDebug(camera, debug);
        // debug.End();
    }

    debug.Shutdown();
    ui.Shutdown();
    app.Shutdown();
    window.Shutdown();

    return 0;
}