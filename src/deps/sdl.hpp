#pragma once

#define SDL_MAIN_HANDLED

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <string>
#include <fmt/format.h>
#include <vector>

namespace sdl
{
    std::string GetError();
    int Init();
    void Quit();
    SDL_Window *CreateWindow(const std::string &title, int w, int h);
    void DestroyWindow(SDL_Window *wnd);

    std::vector<const char *> GetVulkanExtensions(SDL_Window *wnd, bool debug = false);
    bool CreateVulkanSurface(SDL_Window *wnd, VkInstance instance, VkSurfaceKHR *surface);
};

namespace sdl
{
    struct Ticks
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
};