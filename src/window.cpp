#include "window.hpp"

#include "core/all.hpp"
#include <memory>
#include "gl.hpp"
#include "deps/sdl.hpp"
#include "deps/imgui.hpp"
#include "deps/vulkan.hpp"

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
    auto exts = sdl::GetVulkanExtensions(wnd);

    // create instance
    VkInstance instance;

    VkApplicationInfo appInfo;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = title.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = exts.size();
    createInfo.ppEnabledExtensionNames = exts.data();
    createInfo.enabledLayerCount = 0; // later
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    if (result != VK_SUCCESS)
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
    vkDestroyInstance(instance, nullptr);
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

        ImGui_ImplSDL2_ProcessEvent(&event);
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
