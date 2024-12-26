#pragma once

#include "core.hpp"
#include "instance.hpp"

namespace gl
{
    class Surface
    {
    public:
        VkSurfaceKHR handle;
        SDL_Window *window;

        Surface();
        bool Create(const Instance &instance, SDL_Window *window);
        void Destroy(const Instance &instance);
    };
}