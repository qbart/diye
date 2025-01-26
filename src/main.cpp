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
        // experiment->RenderUI(camera, ui);
        ui.Grid(camera);

        ui.EndFrame();

        app.commandBuffers.Reset(app.Frame());
        app.commandBuffers.Begin(app.Frame());
        app.commandBuffers.ClearColor({0.0f, 0.0f, 0.0f, 1.0f});
        app.commandBuffers.ClearDepthStencil();
        app.commandBuffers.CmdBeginRenderPass(app.Frame(), app.renderPass, app.swapChainFramebuffers[app.ImageIndex()], app.swapChain.extent);
        {
            app.uniformBuffersMemory[app.Frame()].CopyRaw(app.device, &app.ubos[app.Frame()], sizeof(app.ubos[app.Frame()]));
            app.commandBuffers.CmdBindGraphicsPipeline(app.Frame(), app.graphicsPipeline);
            app.commandBuffers.CmdViewport(app.Frame(), {0, 0}, app.swapChain.extent);
            app.commandBuffers.CmdScissor(app.Frame(), {0, 0}, app.swapChain.extent);
            app.commandBuffers.CmdBindDescriptorSet(app.Frame(), app.graphicsPipeline, app.descriptorPool.descriptorSets[app.Frame()].handle);
            app.commandBuffers.CmdBindVertexBuffer(app.Frame(), app.vertexBuffer);
            app.commandBuffers.CmdBindIndexBuffer(app.Frame(), app.indexBuffer);
            app.commandBuffers.CmdDrawIndexed(app.Frame(), static_cast<uint32_t>(app.indices.size()));

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
    ui.Shutdown();
    app.Shutdown();
    window.Shutdown();

    return 0;
}