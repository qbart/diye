#include "window.hpp"

#include "core/all.hpp"
#include <memory>
#include "deps/imgui.hpp"

Window::Ptr Window::New(int w, int h, const std::string &title)
{
    if (sdl::Init() != 0)
    {
        fmt::println("Failed to init SDL");
        return nullptr;
    }

    auto wnd = sdl::CreateWindow(title, w, h);
    if (wnd == nullptr)
    {
        fmt::println(sdl::GetError());
        return nullptr;
    }
    vulkan::CreateInstanceInfo info;
    info.title = title;
    info.extensions = sdl::GetVulkanExtensions(wnd, true);
    info.validationLayers = vulkan::CStrings({"VK_LAYER_KHRONOS_validation"});
    vulkan::Instance instance = vulkan::CreateInstance(info, true);
    if (!instance.IsValid())
    {
        fmt::println("Failed to create Vulkan instance");
        return nullptr;
    }

    auto ptr = std::make_shared<Window>();
    ptr->wnd = wnd;
    ptr->instance = instance;
    ptr->size.w = w;
    ptr->size.h = h;
    ptr->isOpen = true;

    return ptr;
}

Window::~Window()
{
    vulkan::DestroyInstance(instance);
    sdl::DestroyWindow(wnd);
    sdl::Quit();
}

void Window::Swap()
{
    SDL_GL_SwapWindow(wnd);
}

void Window::PollEvents()
{
    resized = false;
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
            lastTimeWheeled = SDL_GetTicks();
            break;

        case SDL_WINDOWEVENT_RESIZED:
            size.w = event.window.data1;
            size.h = event.window.data2;
            resized = true;
            break;
        }

        // ImGui_ImplSDL2_ProcessEvent(&event);
    }
}

void Window::Close()
{
    isOpen = false;
}

void Window::Debug()
{
    // glEnable(GL_DEBUG_OUTPUT);
    // gl_printInfo();
    // gl_bindDebugCallback();
}
