#pragma once

#include "glm.hpp"
#include "glfw.hpp"
#include <sstream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <unordered_map>

class UI
{
public:
    UI(GLFWwindow *wnd);
    ~UI();
    void BeginFrame();
    void EndFrame();
    void Draw();
    void Demo();
    void PushFont(uint size);
    void PopFont();

private:
    GLFWwindow *wnd = nullptr;
    ImGuiContext *ptr = nullptr;
    std::unordered_map<uint, uint> fonts;
};