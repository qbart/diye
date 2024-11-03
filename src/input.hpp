#pragma once

#include "core/all.hpp"
#include <unordered_map>
#include "deps/sdl.hpp"

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
    Input(SDL_Window *wnd) : wnd(wnd) {};

    bool KeyReleasedOnce(int key);

    bool KeyPress(int key) const
    {
        return false;
        // return glfwGetKey(wnd, key) == GLFW_PRESS;
    }

    bool KeyRelease(int key) const
    {
        return false;
        // return glfwGetKey(wnd, key) == GLFW_RELEASE;
    }

private:
    std::unordered_map<int, KeyCounter> keyReleaseCount;
    SDL_Window *wnd = nullptr;
};