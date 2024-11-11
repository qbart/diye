#include "experiment.hpp"

int EmptyExperiment::Init()
{
    return 0;
}

void EmptyExperiment::Update(float dt)
{
}

void EmptyExperiment::Render(const Camera &camera, const DebugDrawRenderer &g)
{
}

void EmptyExperiment::RenderUI(UI &ui)
{
}

void EmptyExperiment::Shutdown()
{
}
