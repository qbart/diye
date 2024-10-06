#pragma once

#include "glm.hpp"
#include <unordered_map>
#include <GLFW/glfw3.h>

class Input
{
    struct KeyCounter
    {
        void Reset();
        void Inc();
        bool One();

    private:
        uint value{2};
    };

public:
    Input(GLFWwindow *wnd) : wnd(wnd) {};

    bool KeyReleasedOnce(int key);

    bool KeyPress(int key) const
    {
        return glfwGetKey(wnd, key) == GLFW_PRESS;
    }

    bool KeyRelease(int key) const
    {
        return glfwGetKey(wnd, key) == GLFW_RELEASE;
    }

private:
    std::unordered_map<int, KeyCounter> keyReleaseCount;
    GLFWwindow *wnd = nullptr;
};