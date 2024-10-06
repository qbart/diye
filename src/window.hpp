#pragma once
#include "glfw.hpp"
#include "input.hpp"

class Window
{
public:
    static std::unique_ptr<Window> New(int w, int h, const std::string &title);
    Window() {};
    ~Window();

    bool IsOpen() const { return !glfw.WindowShouldClose(wnd); }
    GLFWwindow *Get() const { return wnd; }
    void Close() { glfw.WindowShouldClose(wnd, true); }
    void Swap() { glfw.SwapBuffers(wnd); }
    void PollEvents() { glfw.PollEvents(); }
    Input GetInput() const { return Input(wnd); }
    void Debug();
    const Dimension& Size() const { return size; }

private:
    Dimension size;
    GLFWwindow *wnd = nullptr;
    GLFW glfw;
};