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
    static std::string GetError()
    {
        return fmt::format("\e[31m{}\e[0m", SDL_GetError());
    }

    static int Init()
    {

        IMG_Init(IMG_INIT_PNG);
        // if (IMG_Init(IMG_INIT_PNG) != 0)
        // {
        //     fmt::println("Failed to init SDL_image");
        //     return nullptr;
        // }

#ifdef SDL_HINT_IME_SHOW_UI
        SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

        int res = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC);
        if (res != 0)
        {
            return res;
        }
        return SDL_Vulkan_LoadLibrary(nullptr);
    }

    static void Quit()
    {
        SDL_Quit();
        IMG_Quit();
    }

    static SDL_Window *CreateWindow(const std::string &title, int w, int h)
    {
        auto flags = SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE;
        return SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, flags);
    }

    static void DestroyWindow(SDL_Window *wnd)
    {
        if (wnd != nullptr)
            SDL_DestroyWindow(wnd);
    }

    static std::vector<const char *> GetVulkanExtensions(SDL_Window *wnd, bool debug = false)
    {
        std::vector<const char *> extensions;

        uint32_t sdlExtensionCount = 0;
        const char **sdlExtensions = nullptr;
        auto sdlRes = SDL_Vulkan_GetInstanceExtensions(wnd, &sdlExtensionCount, nullptr);
        if (sdlRes == SDL_FALSE)
        {
            fmt::print("Failed to get SDL Vulkan extensions count\n");
            return extensions;
        }
        sdlExtensions = new const char *[sdlExtensionCount];
        sdlRes = SDL_Vulkan_GetInstanceExtensions(wnd, &sdlExtensionCount, sdlExtensions);
        if (sdlRes == SDL_FALSE)
        {
            fmt::print("Failed to get SDL Vulkan extensions\n");
            return extensions;
        }
        extensions.reserve(sdlExtensionCount + 1);
        if (debug)
        {
            extensions.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        }
        for (int i = 0; i < sdlExtensionCount; i++)
        {
            extensions.emplace_back(sdlExtensions[i]);
        }
        extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

        delete sdlExtensions;

        return std::move(extensions);
    }
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