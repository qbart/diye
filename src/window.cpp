#include "window.hpp"

#include "core/all.hpp"
#include <memory>
#include "deps/imgui.hpp"
#include "io/binary.hpp"
#include "io/obj.hpp"
#include "deps/fmt.hpp"
#include <array>

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
    sdl::DestroyWindow(wnd);
    sdl::Quit();
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

void Window::PollEvents()
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

        // ImGui_ImplSDL2_ProcessEvent(&event);
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
