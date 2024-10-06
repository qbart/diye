#include "window.hpp"

#include <fmt/core.h>

std::unique_ptr<Window> Window::New(int w, int h, const std::string &title)
{
    auto ptr = std::make_unique<Window>();
    if (!ptr->glfw.Init())
    {
        std::cerr << "Failed to initialize GLFW";
        return nullptr;
    }
    ptr->glfw.WindowHintContextVersion(4, 5);
    ptr->glfw.WindowHintCoreProfileForwardCompat();
    ptr->glfw.WindowHintResizable(true);
    ptr->wnd = ptr->glfw.WindowCreate(w, h, title);
    if (!ptr->wnd)
    {
        std::cerr << "Failed to create Window";
        return nullptr;
    }
    ptr->size.w = w;
    ptr->size.h = h;
    ptr->glfw.MakeContextCurrent(ptr->wnd);
    if (!ptr->glfw.InitGLEW())
    {
        std::cerr << "Failed to init GLEW";
        return false;
    }
    return ptr;
}

Window::~Window()
{
    if (wnd != nullptr)
    {
        glfw.WindowDestroy(wnd);
        wnd = nullptr;
    }
    glfw.Terminate();
}

void Window::Debug()
{
    gl_printInfo();
    gl_bindDebugCallback();
}
