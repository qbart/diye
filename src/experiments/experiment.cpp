#include "experiment.hpp"

int EmptyExperiment::Init(Window *window)
{
    return 0;
}

void EmptyExperiment::Update(float dt)
{
}

void EmptyExperiment::Render(const Camera &camera)
{
}

void EmptyExperiment::RenderDebug(const Camera &camera, const DebugDrawRenderer &g)
{
}

void EmptyExperiment::RenderUI(UI &ui)
{
}

void EmptyExperiment::Shutdown()
{
}
