#pragma once

#include <sstream>
#include <unordered_map>
#include <functional>

#include "../core/all.hpp"
#include "../deps/imgui.hpp"
#include "../gl/app.hpp"
#include "../gl/vulkan.hpp"

enum class ObjectOperation
{
    None,
    Translate,
    Rotate,
    Scale,
};

enum class ObjectTransformMode
{
    Local,
    World,
};

enum class ObjectTransformAxis
{
    XYZ,
    X,
    Y,
    Z,
    XY,
    XZ,
    YZ,
};


class AnimationCurveWidget
{
public:
    std::string Name;
};

static void imgui_check_vk_result(VkResult err);

class UI
{
public:
    struct MouseCallback
    {
        std::function<void()> OnHover;
        std::function<void()> OnDoubleClick;

        MouseCallback() : OnHover([]() {}),
                          OnDoubleClick([]() {})
        {
        }
    };

    struct DragHandleStyle
    {
        float GrabRadius;
        float GrabBorder;
        Vec4 Color;
        Vec4 BorderColor;

        DragHandleStyle() : GrabRadius(5),
                            GrabBorder(2),
                            Color(rgb(96, 0, 255)),
                            BorderColor(rgb(255, 255, 255))
        {
        }
    };

public:
    UI();
    ~UI();
    bool Init(SDL_Window *wnd, const gl::App &app);
    void Shutdown();
    void ProcessEvent(const SDL_Event &event);
    void BeginFrame(const Dimension &size);
    void EndFrame();
    void CmdDraw(VkCommandBuffer cmd);
    void Grid(const Camera &camera);
    void Demo();
    void PushFont(uint size);
    void PopFont();
    std::string ContextMenu(const std::vector<std::string> &items);
    void Text(const Vec2 &pos, const std::string &text, const Vec4 &color = WHITE);
    bool TransformGizmo(const Camera &camera,
                        Transform &transform,
                        ObjectOperation operation = ObjectOperation::None,
                        ObjectTransformMode mode = ObjectTransformMode::World,
                        ObjectTransformAxis axis = ObjectTransformAxis::XYZ);
    bool TranslateGizmo(const Camera &camera, Transform &transform, bool local = false);
    bool RotationGizmo(const Camera &camera, Transform &transform);
    bool ScaleGizmo(const Camera &camera, Transform &transform);
    bool AnimationCurveEditor(AnimationCurve &curve, const AnimationCurveWidget &widget = {"AnimationCurve"});
    bool AnimationCurvePreview(const AnimationCurve &curve);
    bool DragHandle(const std::string &id, const Vec2 &pos, Vec2 &moved, const DragHandleStyle &style = {}, const MouseCallback &callback = {});

private:
    ImVec2 screenPosTo01(const ImVec2 &pos, const ImRect &rect, int precision = 3, bool flipY = false) const;
    ImVec2 screenPosFrom01(const ImVec2 &pos, const ImRect &rect, bool flipY = false) const;
    ImVec2 screenPosFrom01(const Vec2 &pos, const ImRect &rect, bool flipY = false) const;
    ImVec2 screenPosToMappedRect(const ImVec2 &pos, const ImRect &rect, ImRect &mappedRect) const;

private:
    SDL_Window *wnd;
    const gl::App *app;
    gl::DescriptorPool descriptorPool;
    ImGuiContext *ptr;
    std::unordered_map<uint, uint> fonts;
};
