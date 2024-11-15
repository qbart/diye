#pragma once

#include "../core/all.hpp"
#include "../ui.hpp"
#include "../debug_draw_renderer.hpp"
#include "../window.hpp"

class Experiment
{
public:
    Experiment() = default;
    Experiment(const Experiment &) = default;
    Experiment &operator=(const Experiment &) = default;
    Experiment(Experiment &&) = default;
    virtual ~Experiment() = default;
    virtual int Init(Window::Ptr window) = 0;
    virtual void Update(float dt) = 0;
    virtual void Render(const Camera &camera) = 0;
    virtual void RenderDebug(const Camera &camera, const DebugDrawRenderer &g) = 0;
    virtual void RenderUI(UI &ui) = 0;
    virtual void Shutdown() = 0;
};

class EmptyExperiment : public Experiment
{
public:
    int Init(Window::Ptr window) override;
    void Update(float dt) override;
    void Render(const Camera &camera) override;
    void RenderDebug(const Camera &camera, const DebugDrawRenderer &g) override;
    void RenderUI(UI &ui) override;
    void Shutdown() override;
};