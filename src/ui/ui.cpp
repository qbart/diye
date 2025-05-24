#include "ui.hpp"

static void imgui_check_vk_result(VkResult err)
{
    if (err == 0)
        return;

    fmtx::Info("Aborting...");
    char buffer[256];
    sprintf(buffer, "[ui] Error: VkResult = %d", err);
    fmtx::Error(fmt::format("%s", buffer));
    if (err < 0)
        abort();
}

UI::UI() : wnd(nullptr),
           ptr(nullptr),
           app(nullptr)
{
}

UI::~UI()
{
    Shutdown();
}

bool UI::Init(SDL_Window *wnd, const gl::App &app)
{
    this->wnd = wnd;
    this->app = &app;

    ptr = ImGui::CreateContext();
    auto &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();
    auto &style = ImGui::GetStyle();
    auto &colors = style.Colors;

    if (!ImGui_ImplSDL2_InitForVulkan(wnd))
    {
        fmtx::Error("Failed to init ImGui for SDL2");
        return false;
    }

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = app.instance.handle;
    init_info.PhysicalDevice = app.physicalDevice.handle;
    init_info.Device = app.device.handle;
    init_info.Queue = app.device.graphicsQueue.handle;
    init_info.DescriptorPool = app.descriptorPool.handle;
    init_info.MinImageCount = app.MaxFramesInFlight();
    init_info.ImageCount = app.MaxFramesInFlight();
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.RenderPass = app.renderPass.handle;
    init_info.CheckVkResultFn = imgui_check_vk_result;

    if (!ImGui_ImplVulkan_Init(&init_info))
    {
        fmtx::Error("Failed to init ImGui for Vulkan");
        return false;
    }

    if (!ImGui_ImplVulkan_CreateFontsTexture())
    {
        fmtx::Error("Failed to create font texture for ImGui");
        return false;
    }

    app.device.WaitIdle();

    return true;
}

void UI::Shutdown()
{
    if (wnd != nullptr)
    {
        fmtx::Info("Shutting down UI system");
        app->device.WaitIdle();

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL2_Shutdown();
    }

    // Terminate
    if (ptr != nullptr)
    {
        fmtx::Debug("Destroying ImGui context");
        ImGui::DestroyContext();
    }

    ptr = nullptr;
    wnd = nullptr;
}

void UI::ProcessEvent(const SDL_Event &event)
{
    ImGui_ImplSDL2_ProcessEvent(&event);
}

void UI::BeginFrame(const Dimension &size)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
    ImGuizmo::SetRect(0, 0, size.w, size.h);
}

void UI::EndFrame()
{
    // auto &io = ImGui::GetIO();
    ImGui::Render();
    // if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    // {
    //     ImGui::UpdatePlatformWindows();
    //     ImGui::RenderPlatformWindowsDefault();
    // }
}

void UI::CmdDraw(VkCommandBuffer cmd)
{
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
}

void UI::Grid(const Camera &camera)
{
    auto projection = camera.Projection();
    projection[1][1] *= -1;

    ImGuizmo::DrawGrid(glm::value_ptr(camera.View()), glm::value_ptr(projection), glm::value_ptr(glm::mat4(1.f)), 100.f);
}

void UI::Demo()
{
    ImGui::ShowDemoWindow();
}

void UI::PushFont(uint size)
{
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[fonts[size]]);
}

void UI::PopFont()
{
    ImGui::PopFont();
}

std::string UI::ContextMenu(const std::vector<std::string> &items)
{
    if (ImGui::BeginPopupContextWindow("ContextMenu", ImGuiMouseButton_Right))
    {
        for (const auto &item : items)
        {
            if (ImGui::MenuItem(item.c_str()))
            {
                ImGui::EndPopup();
                return item;
            }
        }
        ImGui::EndPopup();
    }
    return "";
}

void UI::Text(const Vec2 &pos, const std::string &text, const Vec4 &color)
{
    ImGui::SetCursorPos(ImVec2(pos.x, pos.y));
    ImGui::TextColored(ImColor(color), text.c_str());
}

bool UI::TranslateGizmo(const Camera &camera, Transform &transform, bool local)
{
    auto projection = camera.Projection();
    projection[1][1] *= -1; // convert back to OpenGL coords

    auto skew = Vec3(0.f);
    auto perspective = Vec4(0.0f);
    auto rotation = transform.rotation;
    auto scale = transform.scale;
    auto mode = ImGuizmo::MODE::WORLD;
    auto mat = transform.ModelMatrix(Transform::Space::WorldOnly);
    Vec3 &translation = transform.position;

    if (local)
    {
        mode = ImGuizmo::MODE::LOCAL;
    }

    bool changed = ImGuizmo::Manipulate(
        glm::value_ptr(camera.View()),
        glm::value_ptr(projection),
        ImGuizmo::OPERATION::TRANSLATE,
        mode,
        glm::value_ptr(mat));
    glm::decompose(mat, scale, rotation, translation, skew, perspective);
    transform.Update();

    return changed;
}

bool UI::RotationGizmo(const Camera &camera, Transform &transform)
{
    auto projection = camera.Projection();
    projection[1][1] *= -1; // convert back to OpenGL coords

    auto mat = transform.ModelMatrix(Transform::Space::World);
    auto skew = Vec3(0.0f);
    auto perspective = Vec4(0.0f);
    auto position = transform.position;
    auto rotation = transform.rotation;
    auto scale = transform.scale;
    bool changed = ImGuizmo::Manipulate(
        glm::value_ptr(camera.View()),
        glm::value_ptr(projection),
        ImGuizmo::OPERATION::ROTATE,
        ImGuizmo::MODE::LOCAL,
        glm::value_ptr(mat));
    glm::decompose(mat, scale, transform.rotation, position, skew, perspective);
    transform.Update();

    return changed;
}

bool UI::ScaleGizmo(const Camera &camera, Transform &transform)
{
    auto projection = camera.Projection();
    projection[1][1] *= -1; // convert back to OpenGL coords

    auto mat = transform.ModelMatrix(Transform::Space::World);
    auto skew = Vec3(0.0f);
    auto perspective = Vec4(0.0f);
    auto rotation = transform.rotation;
    auto position = transform.position;
    bool changed = ImGuizmo::Manipulate(
        glm::value_ptr(camera.View()),
        glm::value_ptr(projection),
        ImGuizmo::OPERATION::SCALE,
        ImGuizmo::MODE::WORLD,
        glm::value_ptr(mat));
    glm::decompose(mat, transform.scale, rotation, position, skew, perspective);
    transform.Update();

    return changed;
}

bool UI::AnimationCurveEditor(AnimationCurve &curve, const AnimationCurveWidget &widget)
{
    // styles
    static ImColor gridCaptionColor(rgb(128, 128, 128));
    static ImColor captionColor(rgb(0, 255, 128));
    static ImColor gridSquareColor(rgb(32, 32, 32));
    static ImColor gridLineColor(rgb(64, 64, 64));
    static ImColor curveColor(rgb(255, 255, 200));
    static Vec4 anchorColor(rgb(255, 192, 0));
    static Vec4 tangentColor(rgb(144, 0, 144));
    static Vec4 inTangentColor(rgb(48, 0, 255));
    static Vec4 outTangentColor(rgb(250, 0, 64));
    static ImColor tangentLineColor(rgb(255, 192, 0));
    static const float curveWidth = 3;
    static const float tangentWidth = 1.5f;

    // config
    static const int resolution = 64;
    static const int precision = 3;
    static const int TN = 10;
    static float T[TN] = {0, 0.05f, 0.1f, 0.15f, 0.2f, 0.25f, 0.3f, 0.35f, 0.4f, 0.45f};
    for (int i = 0; i < TN; ++i)
    {
        T[i] += ImGui::GetIO().DeltaTime * 0.2f;
        if (T[i] > 1.0f)
            T[i] = 0.0f;
    }

    // state
    bool changed = false;
    Vec2 moved(0, 0);
    int selected = -1;
    Vec2 movedInTangent(0, 0);
    int selectedInTangent = -1;
    Vec2 movedOutTangent(0, 0);
    int selectedOutTangent = -1;
    int deleted = -1;
    int tangentSplitJoin = -1;
    int affectedAnchor = -1;
    auto dominantTangent = AnimationCurve::Tangent::Out;
    std::string presetName;
    // ui state
    ImRect bb;
    if (ImGui::Begin(widget.Name.c_str(), nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
    {
        ImVec2 size = ImVec2(600, 600);
        ImGuiWindow *win = ImGui::GetCurrentWindowRead();
        ImRect winbb(win->DC.CursorPos, win->DC.CursorPos + size);
        if (!ImGui::ItemAdd(winbb, NULL))
        {
            ImGui::End(); // win
            return false;
        }
        ImGui::ItemSize(winbb, 0);
        ImDrawList *drawList = ImGui::GetWindowDrawList();
        ImVec2 curveEditorSize = ImVec2(400, 400);

        // draw grid
        auto halfx = (size.x - curveEditorSize.x) / 2;
        auto halfy = (size.y - curveEditorSize.y) / 2;
        bb = ImRect(
            ImVec2(winbb.Min.x + halfx, winbb.Min.y + halfy),
            ImVec2(winbb.Max.x - halfx, winbb.Max.y - halfy));
        ImGui::RenderFrame(bb.Min, bb.Max, gridSquareColor, true, 0.0f);

        float gridStepX = curveEditorSize.x / 4;
        float gridStepY = curveEditorSize.y / 4;

        // draw horizontal grid lines
        for (int x = -gridStepX; x <= curveEditorSize.x + gridStepX; x += gridStepX)
        {
            drawList->AddLine(
                ImVec2(bb.Min.x + x, bb.Min.y - gridStepY),
                ImVec2(bb.Min.x + x, bb.Max.y + gridStepY),
                gridLineColor);
        }
        // draw vertical grid lines
        for (int y = -gridStepY; y <= curveEditorSize.y + gridStepY; y += gridStepY)
        {
            drawList->AddLine(
                ImVec2(bb.Min.x - gridStepX, bb.Min.y + y),
                ImVec2(bb.Max.x + gridStepX, bb.Min.y + y),
                gridLineColor);
        }
        // draw horizontal captions
        for (int x = 0; x <= curveEditorSize.x; x += gridStepX)
        {
            auto pos = screenPosTo01(ImVec2(bb.Min.x + x, bb.Min.y), bb, precision, true);
            drawList->AddText(
                ImVec2(bb.Min.x + x - 5, bb.Min.y - 20),
                gridCaptionColor,
                fmt::format("{}", pos.x).c_str());
        }
        // draw vertical captions
        for (int y = 0; y <= curveEditorSize.y; y += gridStepY)
        {
            auto pos = screenPosTo01(ImVec2(bb.Min.x, bb.Min.y + y), bb, precision, true);
            drawList->AddText(
                ImVec2(bb.Max.x + 5, bb.Min.y + y - 5),
                gridCaptionColor,
                fmt::format("{}", pos.y).c_str());
        }

        // draw curve
        float drawStep = 0;
        float drawStepSize = 0.001f;
        float drawCurveP0;
        float drawCurveP1;
        Vec2 drawCurveStart;
        Vec2 drawCurveEnd;
        while (drawStep + drawStepSize < 1.0)
        {
            float t0 = drawStep;
            float t1 = drawStep + drawStepSize;
            drawCurveP0 = curve.Evaluate(t0);
            drawCurveP1 = curve.Evaluate(t1);
            drawCurveStart = screenPosFrom01(ImVec2(t0, drawCurveP0), bb, true);
            drawCurveEnd = screenPosFrom01(ImVec2(t1, drawCurveP1), bb, true);
            drawList->AddLine(drawCurveStart, drawCurveEnd, curveColor, curveWidth);
            drawStep += drawStepSize;
        }
        drawCurveP0 = drawCurveP1;
        drawCurveP1 = curve.Evaluate(1.0f);
        drawCurveStart = drawCurveEnd;
        drawCurveEnd = screenPosFrom01(ImVec2(1.0f, drawCurveP1), bb, true);
        drawList->AddLine(drawCurveStart, drawCurveEnd, curveColor, curveWidth);

        // ghost anchor that could be added
        ImVec2 mouse = ImGui::GetIO().MousePos;
        auto mouse01 = screenPosTo01(mouse, bb, precision, true);
        if (mouse01.x >= 0 && mouse01.x <= 1 && mouse01.y >= -0.25 && mouse01.y <= 1.25 && !ImGui::IsMouseDragging(0))
        {
            bool allowAdd = true;
            for (int i = 0; i < curve.PointCount(); ++i)
            {
                float distance = Mathf::Distance(Vec2(mouse01.x, mouse01.y), curve.Anchor(i));
                if (distance < 0.05f)
                {
                    auto screenPos = screenPosFrom01(ImVec2(curve[i].Time, curve[i].Value), bb, true);
                    allowAdd = false;
                    break;
                }
                if (curve.HasOutTangent(i))
                {
                    float distance = Mathf::Distance(Vec2(mouse01.x, mouse01.y), curve.OutTangent(i));
                    if (distance < 0.05f)
                    {
                        auto screenPos = screenPosFrom01(ImVec2(curve[i].Time, curve[i].Value), bb, true);
                        allowAdd = false;
                        break;
                    }
                }
                if (curve.HasInTangent(i))
                {
                    float distance = Mathf::Distance(Vec2(mouse01.x, mouse01.y), curve.InTangent(i));
                    if (distance < 0.05f)
                    {
                        auto screenPos = screenPosFrom01(ImVec2(curve[i].Time, curve[i].Value), bb, true);
                        allowAdd = false;
                        break;
                    }
                }
            }
            if (allowAdd)
            {
                drawList->AddLine(ImVec2(mouse.x, bb.Min.y - gridStepY), ImVec2(mouse.x, bb.Max.y + gridStepY), ImColor(anchorColor));
                auto screenPos = screenPosFrom01(ImVec2(mouse01.x, mouse01.y), bb, true);
                drawList->AddCircle(screenPos, 8, ImColor(anchorColor));
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                {
                    curve.AddKey(mouse01.x, mouse01.y);
                    changed = true;
                }
            }
        }

        // moving animation
        for (int i = 0; i < TN; ++i)
        {
            float time = T[i];
            auto value = curve.Evaluate(time);
            drawList->AddCircleFilled(ImVec2(time * (bb.Max.x - bb.Min.x) + bb.Min.x, (1 - value) * (bb.Max.y - bb.Min.y) + bb.Min.y), 5, curveColor);
        }

        // anchors and tangents with lines and text
        // tangent lines
        for (int i = 0; i < curve.PointCount(); ++i)
        {
            if (curve.HasOutTangent(i))
            {
                const auto anchorPos = screenPosFrom01(Vec2(curve[i].Time, curve[i].Value), bb, true);
                const auto tangentPos = screenPosFrom01(Vec2(curve.OutTangent(i)), bb, true);
                drawList->AddLine(ImVec2(anchorPos.x, anchorPos.y), ImVec2(tangentPos.x, tangentPos.y), tangentLineColor, tangentWidth);
            }
            if (curve.HasInTangent(i))
            {
                const auto anchorPos = screenPosFrom01(Vec2(curve[i].Time, curve[i].Value), bb, true);
                const auto outTangentPos = screenPosFrom01(curve.InTangent(i), bb, true);
                drawList->AddLine(ImVec2(anchorPos.x, anchorPos.y), ImVec2(outTangentPos.x, outTangentPos.y), tangentLineColor, tangentWidth);
            }
        }

        // tangents only
        ImGui::PushID("tangents");
        for (int i = 0; i < curve.PointCount(); ++i)
        {
            ImGui::PushID(i);
            const auto outTangentPos = screenPosFrom01(curve.OutTangent(i), bb, true);
            const auto inTangentPos = screenPosFrom01(curve.InTangent(i), bb, true);

            MouseCallback callback;
            callback.OnHover = [i, &affectedAnchor]()
            {
                affectedAnchor = i;
                // drawList->AddText(ImVec2(outTangentPos.x - 10, outTangentPos.y - 25),
                //                   captionColor,
                //                   fmt::format("OutTangent: {}", curve[i].OutTangent).c_str());
                // drawList->AddText(ImVec2(outTangentPos.x - 10, outTangentPos.y - 45),
                //                   captionColor,
                //                   fmt::format("InTangent: {}", curve[i].InTangent).c_str());
            };

            DragHandleStyle dragHandleStyle;
            dragHandleStyle.Color = tangentColor;
            dragHandleStyle.GrabRadius = 4;
            dragHandleStyle.GrabBorder = 1;
            if (curve.HasOutTangent(i))
            {
                callback.OnDoubleClick = [i, &tangentSplitJoin, &dominantTangent]()
                {
                    tangentSplitJoin = i;
                    dominantTangent = AnimationCurve::Tangent::Out;
                };
                callback.OnHover = [&]()
                {
                    auto pos = screenPosFrom01(curve.OutTangent(i), bb, true);
                    drawList->AddText(ImVec2(pos.x - 10, pos.y - 25), captionColor, fmt::format("{}", curve[i].OutTangent).c_str());
                };
                if (!curve[i].Locked)
                    dragHandleStyle.Color = outTangentColor;
                if (DragHandle("AnimationCurveDragTangentOut", outTangentPos, movedOutTangent, dragHandleStyle, callback))
                {
                    selectedOutTangent = i;
                    affectedAnchor = i;
                }
            }
            if (curve.HasInTangent(i))
            {
                callback.OnDoubleClick = [i, &tangentSplitJoin, &dominantTangent]()
                {
                    tangentSplitJoin = i;
                    dominantTangent = AnimationCurve::Tangent::In;
                };
                callback.OnHover = [&]()
                {
                    auto pos = screenPosFrom01(curve.InTangent(i), bb, true);
                    drawList->AddText(ImVec2(pos.x - 10, pos.y - 25), captionColor, fmt::format("{}", curve[i].OutTangent).c_str());
                };
                if (!curve[i].Locked)
                    dragHandleStyle.Color = inTangentColor;
                if (DragHandle("AnimationCurveDragTangentIn", inTangentPos, movedInTangent, dragHandleStyle, callback))
                {
                    selectedInTangent = i;
                    affectedAnchor = i;
                }
            }
            ImGui::PopID(); // i
        }
        ImGui::PopID(); // tangents

        // anchors only
        ImGui::PushID("anchors");
        for (int i = 0; i < curve.PointCount(); ++i)
        {
            ImGui::PushID(i);
            auto pos = screenPosFrom01(curve.Anchor(i), bb, true);

            MouseCallback callback;
            callback.OnHover = [i, &pos, &curve, drawList]()
            {
                drawList->AddText(ImVec2(pos.x - 10, pos.y - 25),
                                  captionColor,
                                  fmt::format("{},{}", curve[i].Time, curve[i].Value).c_str());
            };
            callback.OnDoubleClick = [i, &deleted]()
            {
                deleted = i;
            };

            DragHandleStyle dragHandleStyle;
            dragHandleStyle.Color = anchorColor;
            dragHandleStyle.GrabRadius = affectedAnchor == i ? 1 : 5;
            dragHandleStyle.GrabBorder = affectedAnchor == i ? 0 : 2;
            if (DragHandle("AnimationCurveDrag", pos, moved, dragHandleStyle, callback))
            {
                selected = i;
            }
            // debug index
            // drawList->AddText(ImVec2(x - 10, y - 8),
            //                   ImColor(1.0f, 0.0f, 0.0f),
            //                   fmt::format("[{}]", i).c_str());
            ImGui::PopID(); // i
        }
        ImGui::PopID(); // anchors

        presetName = ContextMenu({"Zero", "One", "Linear", "EaseIn", "EaseOut", "EaseInOut"});
    }

    ImGui::End(); // win

    // only one operation at a time
    if (presetName != "")
    {
        if (presetName == "Zero")
            curve.ApplyPreset(AnimationCurve::Preset::Zero);
        else if (presetName == "One")
            curve.ApplyPreset(AnimationCurve::Preset::One);
        else if (presetName == "Linear")
            curve.ApplyPreset(AnimationCurve::Preset::Linear);
        else if (presetName == "EaseIn")
            curve.ApplyPreset(AnimationCurve::Preset::EaseIn);
        else if (presetName == "EaseOut")
            curve.ApplyPreset(AnimationCurve::Preset::EaseOut);
        else if (presetName == "EaseInOut")
            curve.ApplyPreset(AnimationCurve::Preset::EaseInOut);

        changed = true;
    }
    else if (selected != -1)
    {
        auto keyframe = screenPosTo01(moved, bb, precision, true);
        curve.SetPoint(selected, keyframe.x, keyframe.y);
        changed = true;
    }
    else if (selectedInTangent != -1)
    {
        auto keyframe = screenPosTo01(movedInTangent, bb, precision, true);
        curve.SetInTangent(selectedInTangent, keyframe.x, keyframe.y);
        changed = true;
    }
    else if (selectedOutTangent != -1)
    {
        auto keyframe = screenPosTo01(movedOutTangent, bb, precision, true);
        curve.SetOutTangent(selectedOutTangent, keyframe.x, keyframe.y);
        changed = true;
    }
    else if (deleted != -1)
    {
        curve.RemoveKeyframe(deleted);
        changed = true;
    }
    else if (tangentSplitJoin != -1)
    {
        curve.ToggleTangentSplitJoin(tangentSplitJoin, dominantTangent);
        changed = true;
    }
    return changed;
}

bool UI::AnimationCurvePreview(const AnimationCurve &curve)
{
    static const ImColor curveColor(rgb(255, 255, 200));
    static ImColor gridSquareColor(rgb(32, 32, 32));
    static const float curveWidth = 1;
    bool changed = false;

    ImGuiWindow *win = ImGui::GetCurrentWindow();
    if (win->SkipItems)
        return false;

    ImVec2 previewSize = ImVec2(70, 70);
    ImRect winbb(win->DC.CursorPos, win->DC.CursorPos + previewSize);
    if (!ImGui::ItemAdd(winbb, NULL))
        return false;
    ImGui::ItemSize(winbb, 0);

    ImRect bb(
        ImVec2(winbb.Min.x + 10, winbb.Min.y + 10),
        ImVec2(winbb.Max.x - 10, winbb.Max.y - 10));
    ImGui::PushClipRect(winbb.Min, winbb.Max, true);

    ImGui::RenderFrame(winbb.Min, winbb.Max, gridSquareColor, false, 0.0f);
    auto drawList = ImGui::GetWindowDrawList();
    float drawStep = 0;
    float drawStepSize = 0.01f;
    float drawCurveP0;
    float drawCurveP1;
    Vec2 drawCurveStart;
    Vec2 drawCurveEnd;
    while (drawStep + drawStepSize < 1.0)
    {
        float t0 = drawStep;
        float t1 = drawStep + drawStepSize;
        drawCurveP0 = curve.Evaluate(t0);
        drawCurveP1 = curve.Evaluate(t1);
        drawCurveStart = screenPosFrom01(ImVec2(t0, drawCurveP0), bb, true);
        drawCurveEnd = screenPosFrom01(ImVec2(t1, drawCurveP1), bb, true);
        drawList->AddLine(drawCurveStart, drawCurveEnd, curveColor, curveWidth);
        drawStep += drawStepSize;
    }
    drawCurveP0 = drawCurveP1;
    drawCurveP1 = curve.Evaluate(1.0f);
    drawCurveStart = drawCurveEnd;
    drawCurveEnd = screenPosFrom01(ImVec2(1.0f, drawCurveP1), bb, true);
    drawList->AddLine(drawCurveStart, drawCurveEnd, curveColor, curveWidth);
    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        changed = true;
    ImGui::PopClipRect();

    return changed;
}

bool UI::DragHandle(const std::string &id, const Vec2 &pos, Vec2 &moved, const UI::DragHandleStyle &style, const UI::MouseCallback &callback)
{
    bool changed = false;
    ImVec2 handlePos = ImVec2(pos.x, pos.y);
    ImDrawList *drawList = ImGui::GetWindowDrawList();

    auto buttonPos = ImVec2(
        handlePos.x - style.GrabRadius,
        handlePos.y - style.GrabRadius);

    auto cursor = ImGui::GetCursorScreenPos();
    bool triggeredHover = false;
    ImGui::SetCursorScreenPos(buttonPos);
    ImGui::InvisibleButton(("@DragHandle__" + id).c_str(), ImVec2(style.GrabRadius * 2, style.GrabRadius * 2));
    ImGui::SetCursorScreenPos(cursor);

    if (ImGui::IsItemHovered())
        triggeredHover = true;

    float alpha = style.Color.w;
    float borderAlpha = style.BorderColor.w;
    if (ImGui::IsItemActive() || ImGui::IsItemHovered())
    {
        alpha = style.Color.w * 0.8f;
        borderAlpha = style.BorderColor.w * 0.8f;
    }
    if (style.GrabBorder > 0)
        drawList->AddCircleFilled(handlePos,
                                  style.GrabRadius * 2,
                                  ImColor(style.BorderColor.x, style.BorderColor.y, style.BorderColor.z, borderAlpha));

    drawList->AddCircleFilled(handlePos,
                              style.GrabRadius * 2 - style.GrabBorder,
                              ImColor(style.Color.x, style.Color.y, style.Color.z, alpha));

    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
    {
        auto delta = ImGui::GetIO().MouseDelta;
        moved = Vec2(handlePos.x + delta.x, handlePos.y + delta.y);
        changed = true;
        // drawList->AddText(ImVec2(moved.x - 10, moved.y - 10), ImColor(1.0f, 1.0f, 1.0f), fmt::format("{},{}", moved.x, moved.y).c_str());
        triggeredHover = true;
    }
    if (triggeredHover)
        callback.OnHover();
    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        callback.OnDoubleClick();

    return changed;
}

ImVec2 UI::screenPosTo01(const ImVec2 &pos, const ImRect &rect, int precision, bool flipY) const
{
    float mappedX = (pos.x - rect.Min.x) / (rect.Max.x - rect.Min.x);
    float mappedY = (pos.y - rect.Min.y) / (rect.Max.y - rect.Min.y);
    if (flipY)
        mappedY = 1 - mappedY;

    mappedX = roundf(mappedX * pow(10, precision)) / pow(10, precision);
    mappedY = roundf(mappedY * pow(10, precision)) / pow(10, precision);

    return ImVec2(mappedX, mappedY);
}

ImVec2 UI::screenPosFrom01(const ImVec2 &pos, const ImRect &rect, bool flipY) const
{
    float y = pos.y;
    if (flipY)
        y = 1 - pos.y;

    return ImVec2(
        pos.x * (rect.Max.x - rect.Min.x) + rect.Min.x,
        y * (rect.Max.y - rect.Min.y) + rect.Min.y);
}

ImVec2 UI::screenPosFrom01(const Vec2 &pos, const ImRect &rect, bool flipY) const
{
    return screenPosFrom01(ImVec2(pos.x, pos.y), rect, flipY);
}

ImVec2 UI::screenPosToMappedRect(const ImVec2 &pos, const ImRect &rect, ImRect &mappedRect) const
{
    float mappedX = (pos.x - rect.Min.x) / (rect.Max.x - rect.Min.x);
    float mappedY = (pos.y - rect.Min.y) / (rect.Max.y - rect.Min.y);

    return ImVec2(
        mappedRect.Min.x + mappedX * (mappedRect.Max.x - mappedRect.Min.x),
        mappedRect.Min.y + mappedY * (mappedRect.Max.y - mappedRect.Min.y));
}
