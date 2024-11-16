#include "window.hpp"

#include "core/all.hpp"
#include <memory>
#include "gl.hpp"
#include "deps/sdl.hpp"
#include "deps/imgui.hpp"

Window::Ptr Window::New(int w, int h, const std::string &title)
{
    IMG_Init(IMG_INIT_PNG);
    // if (IMG_Init(IMG_INIT_PNG) != 0)
    // {
    //     fmt::println("Failed to init SDL_image");
    //     return nullptr;
    // }
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC) != 0)
    {
        fmt::println("Failed to init SDL");
        return nullptr;
    }

#ifdef __APPLE__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
#endif

    uint8 contextFlags = SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;
#ifdef _DEBUG
    contextFlags |= SDL_GL_CONTEXT_DEBUG_FLAG;
#endif
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, contextFlags);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

    auto flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;
    flags |= SDL_WINDOW_RESIZABLE;

    auto wnd = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, flags);
    if (wnd == nullptr)
    {
        fmt::println("Failed to create window");
        return nullptr;
    }
    auto glContext = SDL_GL_CreateContext(wnd);
    if (glContext == nullptr)
    {
        fmt::println("Failed to create GL context");
        return nullptr;
    }

    auto ptr = std::make_shared<Window>();
    ptr->wnd = wnd;
    ptr->glContext = glContext;
    ptr->size.w = w;
    ptr->size.h = h;
    ptr->isOpen = true;
    SDL_GL_MakeCurrent(ptr->wnd, ptr->glContext);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        fmt::println("Failed to init GLEW");
        return nullptr;
    }
    return ptr;
}

Window::~Window()
{
    if (glContext != nullptr)
    {
        SDL_GL_MakeCurrent(wnd, nullptr);
        SDL_GL_DeleteContext(glContext);
        glContext = nullptr;
    }
    if (wnd != nullptr)
    {
        SDL_DestroyWindow(wnd);
        wnd = nullptr;
    }
    SDL_Quit();
    IMG_Quit();
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

void Window::Close()
{
    isOpen = false;
}

void Window::Debug()
{
    glEnable(GL_DEBUG_OUTPUT);
    gl_printInfo();
    gl_bindDebugCallback();
}
