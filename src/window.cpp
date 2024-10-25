#include "window.hpp"

#include "core/all.hpp"
#include <memory>
#include "sdl.hpp"

std::unique_ptr<Window> Window::New(int w, int h, const std::string &title)
{
    IMG_Init(IMG_INIT_PNG);
    auto ptr = std::make_unique<Window>();
    if (!ptr->glfw.Init())
    {
        std::cerr << "Failed to initialize GLFW";
        return nullptr;
    }
#ifdef __APPLE__
    ptr->glfw.WindowHintContextVersion(4, 1);
#else
    ptr->glfw.WindowHintContextVersion(4, 5);
#endif
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
        return nullptr;
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
    IMG_Quit();
}

void Window::Debug()
{
	glEnable(GL_DEBUG_OUTPUT);
    gl_printInfo();
    gl_bindDebugCallback();
}
