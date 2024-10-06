#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "gl.hpp"

struct GLFW
{
    bool Init()
    {
        return glfwInit();
    }
    bool InitGLEW()
    {
        glewExperimental = GL_TRUE;
        return glewInit() == GLEW_OK;
    }
    void Terminate() { glfwTerminate(); }
    void PollEvents() { glfwPollEvents(); }

    void WindowHintContextVersion(int major, int minor)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
    }

    void WindowHintCoreProfileForwardCompat()
    {
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    }

    void WindowHintResizable(bool resizable)
    {
        glfwWindowHint(GLFW_RESIZABLE, static_cast<int>(resizable));
    }

    bool WindowShouldClose(GLFWwindow *wnd) const
    {
        return glfwWindowShouldClose(wnd);
    }

    void WindowShouldClose(GLFWwindow *wnd, bool value)
    {
        glfwSetWindowShouldClose(wnd, static_cast<int>(value));
    }

    void MakeContextCurrent(GLFWwindow *wnd)
    {
        glfwMakeContextCurrent(wnd);
    }

    void SwapBuffers(GLFWwindow *wnd)
    {
        glfwSwapBuffers(wnd);
    }

    GLFWwindow *WindowCreate(int w, int h, std::string title)
    {
        return glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr);
    }

    void WindowDestroy(GLFWwindow *wnd)
    {
        glfwDestroyWindow(wnd);
    }

    Dimension FramebufferSize(GLFWwindow *wnd) const
    {
        int width, height;
        glfwGetFramebufferSize(wnd, &width, &height);
        return {width, height};
    }

    void WindowSetPosition(GLFWwindow *wnd, const Vec2 &pos)
    {
        glfwSetWindowPos(wnd, pos.x, pos.y);
    }

    void WindowSetSize(GLFWwindow *wnd, const Dimension &dim)
    {
        glfwSetWindowSize(wnd, dim.w, dim.h);
    }

    void WindowResizeCallback(GLFWwindow *wnd, GLFWwindowsizefun cbfun)
    {
        glfwSetWindowSizeCallback(wnd, cbfun);
    }
};