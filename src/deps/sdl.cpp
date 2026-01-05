#include "sdl.hpp"
#include <vulkan/vulkan_beta.h>
#include <memory>
#include "imgui.hpp"
#include "fmt.hpp"
#include <array>

#include "../ui/ui.hpp"

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

    bool CreateVulkanSurface(SDL_Window *wnd, VkInstance instance, VkSurfaceKHR *surface)
    {
        return SDL_Vulkan_CreateSurface(wnd, instance, surface) == SDL_TRUE;
    }

    VkExtent2D GetVulkanFramebufferSize(SDL_Window *window)
    {
        VkExtent2D extent;
        int w, h;
        SDL_Vulkan_GetDrawableSize(window, &w, &h);
        extent.width = static_cast<uint32_t>(w);
        extent.height = static_cast<uint32_t>(h);

        return extent;
    }
};

namespace sdl
{
    Window::Window() : active(false),
                       isOpen(false),
                       size({0, 0}),
                       wnd(nullptr),
                       mousePos(0, 0),
                       mouseRelPos(0, 0),
                       mouseWheel(0, 0),
                       lastTimeWheeled(0),
                       resized(false)
    {
    }

    Window::~Window()
    {
        Shutdown();
    }

    bool Window::Init(int w, int h, const std::string &title)
    {
        if (sdl::Init() != 0)
        {
            fmtx::Error("Failed to init SDL");
            return false;
        }

        wnd = sdl::CreateWindow(title, w, h);
        if (wnd == nullptr)
        {
            fmtx::Error(sdl::GetError());
            return false;
        }
        size.w = w;
        size.h = h;
        isOpen = true;

        return true;
    }

    void Window::Shutdown()
    {
        if (wnd != nullptr)
        {
            fmtx::Info("Shutting down window");

            sdl::DestroyWindow(wnd);
            sdl::Quit();
            wnd = nullptr;
        }
    }

    void Window::PollEvents(UI *const ui)
    {
        SetResized(false);
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                isOpen = false;
                break;

            case SDL_KEYDOWN:
                inputs[event.key.keysym.sym] = true;
                break;

            case SDL_KEYUP:
                inputs[event.key.keysym.sym] = false;
                break;

            case SDL_MOUSEMOTION:
                mousePos.x = event.motion.x;
                mousePos.y = event.motion.y;
                mouseRelPos.x = event.motion.xrel;
                mouseRelPos.y = event.motion.yrel;
                break;

            case SDL_MOUSEBUTTONDOWN:
                mouseInputs[event.button.button] = true;
                break;

            case SDL_MOUSEBUTTONUP:
                mouseInputs[event.button.button] = false;
                break;

            case SDL_MOUSEWHEEL:
                mouseWheel.x = event.wheel.preciseX;
                mouseWheel.y = event.wheel.preciseY;
                lastTimeWheeled = SDL_GetTicks64();
                break;

            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    fmtx::Debug("[sdl] Window resized");
                    size.w = event.window.data1;
                    size.h = event.window.data2;
                    resized = true;
                }
                if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
                {
                    fmtx::Debug("[sdl] Window focus gained");
                    active = true;
                }
                else if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
                {
                    fmtx::Debug("[sdl] Window focus lost");
                    active = false;
                }
                break;
            }

            if (ui != nullptr)
                ui->ProcessEvent(event);
        }
    }

    void Window::Close()
    {
        isOpen = false;
        active = false;
    }

    bool Window::KeyJustReleased(int key)
    {
        if (KeyDown(key))
            wasDown[key] = true;
        else if (wasDown.find(key) != wasDown.end() && wasDown[key])
        {
            wasDown[key] = false;
            return true;
        }

        return false;
    }

    bool Window::KeyDown(int key)
    {
        const auto val = inputs.find(key);
        if (val != inputs.end())
        {
            return val->second;
        }
        return false;
    }

    bool Window::MouseButtonDown(uint8 button) const
    {
        const auto val = mouseInputs.find(button);
        if (val != mouseInputs.end())
        {
            return val->second;
        }
        return false;
    }

    bool Window::MouseButtonUp(uint8 button)
    {
        if (MouseButtonDown(button))
            mouseWasDown[button] = true;
        else if (mouseWasDown.find(button) != mouseWasDown.end() && mouseWasDown[button])
        {
            mouseWasDown[button] = false;
            return true;
        }

        return false;
    }

    void Window::FreeCameraControls(Camera &camera, float dt)
    {
        if (KeyJustReleased(SDLK_ESCAPE))
            Close();

        if (KeyDown(SDLK_w))
            camera.MoveForward(5 * dt);

        if (KeyDown(SDLK_s))
            camera.MoveBackward(5 * dt);

        if (KeyDown(SDLK_a))
            camera.MoveLeft(5 * dt);

        if (KeyDown(SDLK_d))
            camera.MoveRight(5 * dt);

        if (MouseButtonDown(SDL_BUTTON_RIGHT) && KeyDown(SDLK_LALT))
        {
            auto md = MouseRelativePosition();
            camera.OrbitAround(UP, ZERO, md.x * 2 * dt);
            camera.OrbitAround(LEFT, ZERO, -md.y * 2 * dt);
        }
        else if (MouseButtonDown(SDL_BUTTON_RIGHT))
        {
            auto md = MouseRelativePosition();
            camera.LookAround(md.y * 10 * dt, md.x * 10 * dt);
        }
        if (MouseWheelScrolled())
        {
            auto mw = MouseWheel();
            camera.MoveForward(MouseWheel().y * dt * 5);
        }
    }

}
