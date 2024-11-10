#pragma once

#define SDL_MAIN_HANDLED

#include <SDL.h>
#include <SDL_image.h>

struct SDLTicks
{
public:
    inline void Update()
    {
        ticks = SDL_GetTicks64();
        last = now;
        now = ticks;
        dt = (now - last) / 1000.0;
    }
    inline double DeltaTime()
    {
        return dt;
    }
    inline double TotalMilliseconds()
    {
        return ticks / 1000.0;
    }

private:
    double dt = 0;
    double last = 0;
    double now = 0;
    double ticks = 0;
};