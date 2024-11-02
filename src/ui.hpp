#pragma once

#include <sstream>
#include <unordered_map>
#include <functional>

#include "core/all.hpp"
#include "glfw.hpp"
#include "deps/imgui.hpp"

class UI
{
public:
    struct MouseCallback
    {
        std::function<void()> OnHover = []() {};
        std::function<void()> OnDoubleClick = []() {};
    };

    struct DragHandleStyle
    {
        float GrabRadius = 5;
        float GrabBorder = 2;
        Vec4 Color = rgb(96, 0, 255);
        Vec4 BorderColor = rgb(255, 255, 255);
    };

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
    bool TranslateGizmo(const Camera &camera, Transform &transform, bool local = false);
    bool RotationGizmo(const Camera &camera, Transform &transform);
    bool ScaleGizmo(const Camera &camera, Transform &transform);
    bool AnimationCurveWidget(AnimationCurve &curve);
    bool DragHandle(const std::string &id, const Vec2 &pos, Vec2 &moved, const DragHandleStyle &style = {}, const MouseCallback &callback = {});

private:
    ImVec2 screenPosTo01(const ImVec2 &pos, const ImRect &rect, int precision = 3, bool flipY = false) const;
    ImVec2 screenPosFrom01(const ImVec2 &pos, const ImRect &rect, bool flipY = false) const;
    ImVec2 screenPosToMappedRect(const ImVec2 &pos, const ImRect &rect, ImRect &mappedRect) const;
    void drawRecursive(const AnimationCurve &curve, float t0, float t1, Vec2 p0, Vec2 p1, float tolerance, int maxSteps, std::function<void(const Vec2 &, const Vec2 &)> drawFn);

private:
    GLFWwindow *wnd = nullptr;
    ImGuiContext *ptr = nullptr;
    std::unordered_map<uint, uint> fonts;
};