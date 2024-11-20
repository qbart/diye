#include "sdl.hpp"
#include <vulkan/vulkan_beta.h>

namespace sdl
{
    std::string GetError()
    {
        return SDL_GetError();
    }

    int Init()
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

    void Quit()
    {
        SDL_Quit();
        IMG_Quit();
    }

    SDL_Window *CreateWindow(const std::string &title, int w, int h)
    {
        auto flags = SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE;
        return SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, flags);
    }

    void DestroyWindow(SDL_Window *wnd)
    {
        if (wnd != nullptr)
            SDL_DestroyWindow(wnd);
    }

    std::vector<const char *> GetVulkanExtensions(SDL_Window *wnd, bool debug)
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
        for (int i = 0; i < sdlExtensionCount; i++)
        {
            extensions.emplace_back(sdlExtensions[i]);
        }
        extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        if (debug)
        {
            extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        delete[] sdlExtensions;

        return std::move(extensions);
    }
};
