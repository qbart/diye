#pragma once

#include <sstream>
#include <unordered_map>
#include <functional>

#include "core/all.hpp"
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
    UI(SDL_Window *wnd);
    ~UI();
    void BeginFrame(const Dimension &size);
    void EndFrame();
    void Draw();
    void Grid(const Camera &camera);
    void Demo();
    void PushFont(uint size);
    void PopFont();
    std::string ContextMenu(const std::vector<std::string> &items);
    void Text(const Vec2 &pos, const std::string &text, const Vec4 &color = WHITE);
    bool TranslateGizmo(const Camera &camera, Transform &transform, bool local = false);
    bool RotationGizmo(const Camera &camera, Transform &transform);
    bool ScaleGizmo(const Camera &camera, Transform &transform);
    bool AnimationCurveWidget(AnimationCurve &curve);
    bool AnimationCurvePreview(const AnimationCurve &curve);
    bool DragHandle(const std::string &id, const Vec2 &pos, Vec2 &moved, const DragHandleStyle &style = {}, const MouseCallback &callback = {});

private:
    ImVec2 screenPosTo01(const ImVec2 &pos, const ImRect &rect, int precision = 3, bool flipY = false) const;
    ImVec2 screenPosFrom01(const ImVec2 &pos, const ImRect &rect, bool flipY = false) const;
    ImVec2 screenPosFrom01(const Vec2 &pos, const ImRect &rect, bool flipY = false) const;
    ImVec2 screenPosToMappedRect(const ImVec2 &pos, const ImRect &rect, ImRect &mappedRect) const;

private:
    SDL_Window *wnd = nullptr;
    ImGuiContext *ptr = nullptr;
    std::unordered_map<uint, uint> fonts;
};