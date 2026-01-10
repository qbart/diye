#pragma once

#include "../core/all.hpp"
#include "../deps/sdl.hpp"
#include "../ui/ui.hpp"

class Experiment
{
public:
    Experiment()                                        = default;
    Experiment(const Experiment &)                      = default;
    Experiment &operator=(const Experiment &)           = default;
    Experiment(Experiment &&)                           = default;
    virtual ~Experiment()                               = default;
    virtual int Init(sdl::Window *window)               = 0;
    virtual void Update(float dt)                       = 0;
    virtual void Render(const Camera &camera)           = 0;
    virtual void RenderDebug(const Camera &camera)      = 0;
    virtual void RenderUI(const Camera &camera, UI &ui) = 0;
    virtual void Shutdown()                             = 0;
};

class EmptyExperiment : public Experiment
{
public:
    int Init(sdl::Window *window) override;
    void Update(float dt) override;
    void Render(const Camera &camera) override;
    void RenderDebug(const Camera &camera) override;
    void RenderUI(const Camera &camera, UI &ui) override;
    void Shutdown() override;
};