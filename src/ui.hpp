#pragma once

#include "glm.hpp"
#include "glfw.hpp"
#include <sstream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <ImGuizmo.h>
#include <unordered_map>
#include "camera.hpp"

class UI
{
public:
    UI(GLFWwindow *wnd);
    ~UI();
    void BeginFrame(const Dimension &size);
    void EndFrame();
    void Draw();
    void Grid(const Camera &camera);
    void Demo();
    void PushFont(uint size);
    void PopFont();
    void TranslateGizmo(const Camera &camera, Transform &transform, bool local = false);
    void RotationGizmo(const Camera &camera, Transform &transform);
    void ScaleGizmo(const Camera &camera, Transform &transform);

private:
    GLFWwindow *wnd = nullptr;
    ImGuiContext *ptr = nullptr;
    std::unordered_map<uint, uint> fonts;
};