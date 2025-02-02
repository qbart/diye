#include "experiment.hpp"

int EmptyExperiment::Init(sdl::Window *window)
{
    return 0;
}

void EmptyExperiment::Update(float dt)
{
}

void EmptyExperiment::Render(const Camera &camera)
{
}

void EmptyExperiment::RenderDebug(const Camera &camera)
{
}

void EmptyExperiment::RenderUI(const Camera &camera, UI &ui)
{
}

void EmptyExperiment::Shutdown()
{
}
