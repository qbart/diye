#include "ui.hpp"

UI::UI(GLFWwindow *wnd) : wnd(wnd)
{
    ptr = ImGui::CreateContext();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    auto &style = ImGui::GetStyle();
    auto &colors = style.Colors;

    style.ScaleAllSizes(2);

    // auto robotopath = std::string(FONTS_PATH) + "Roboto-Regular.ttf";
    // auto io = ctx.io();
    // io.Fonts->AddFontFromFileTTF(robotopath.c_str(), 24);
    // io.Fonts->AddFontFromFileTTF(robotopath.c_str(), 16);
    // ctx.fonts[24] = 0;
    // ctx.fonts[16] = 1;

    ImGui_ImplGlfw_InitForOpenGL(wnd, true);
#ifdef __APPLE__
    ImGui_ImplOpenGL3_Init("#version 410");
#else
    ImGui_ImplOpenGL3_Init("#version 450");
#endif
}

UI::~UI()
{
    // Terminate
    if (ptr != nullptr)
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        ptr = nullptr;
    }
}

void UI::BeginFrame(const Dimension &size)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
    ImGuizmo::SetRect(0, 0, size.w, size.h);
}

void UI::EndFrame()
{
    ImGui::Render();
}

void UI::Draw()
{
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UI::Grid(const Camera &camera)
{
    ImGuizmo::DrawGrid(glm::value_ptr(camera.GetViewMatrix()), glm::value_ptr(camera.GetProjection()), glm::value_ptr(glm::mat4(1.f)), 100.f);
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

bool UI::TranslateGizmo(const Camera &camera, Transform &transform, bool local)
{
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
        glm::value_ptr(camera.GetViewMatrix()),
        glm::value_ptr(camera.GetProjection()),
        ImGuizmo::OPERATION::TRANSLATE,
        mode,
        glm::value_ptr(mat));
    glm::decompose(mat, scale, rotation, translation, skew, perspective);
    transform.Update();

    return changed;
}

bool UI::RotationGizmo(const Camera &camera, Transform &transform)
{
    auto mat = transform.ModelMatrix(Transform::Space::World);
    auto skew = Vec3(0.0f);
    auto perspective = Vec4(0.0f);
    auto position = transform.position;
    auto rotation = transform.rotation;
    auto scale = transform.scale;
    bool changed = ImGuizmo::Manipulate(
        glm::value_ptr(camera.GetViewMatrix()),
        glm::value_ptr(camera.GetProjection()),
        ImGuizmo::OPERATION::ROTATE,
        ImGuizmo::MODE::LOCAL,
        glm::value_ptr(mat));
    glm::decompose(mat, scale, transform.rotation, position, skew, perspective);
    transform.Update();

    return changed;
}

bool UI::ScaleGizmo(const Camera &camera, Transform &transform)
{
    auto mat = transform.ModelMatrix(Transform::Space::World);
    auto skew = Vec3(0.0f);
    auto perspective = Vec4(0.0f);
    auto rotation = transform.rotation;
    auto position = transform.position;
    bool changed = ImGuizmo::Manipulate(
        glm::value_ptr(camera.GetViewMatrix()),
        glm::value_ptr(camera.GetProjection()),
        ImGuizmo::OPERATION::SCALE,
        ImGuizmo::MODE::WORLD,
        glm::value_ptr(mat));
    glm::decompose(mat, transform.scale, rotation, position, skew, perspective);
    transform.Update();

    return changed;
}

bool UI::AnimationCurveWidget(AnimationCurve &curve)
{
    using namespace ImGui;

    // styles
    static ImColor gridCaptionColor(rgb(128, 128, 128));
    static ImColor captionColor(rgb(0, 255, 128));
    static ImColor gridSquareColor(rgb(32, 32, 32));
    static ImColor gridLineColor(rgb(64, 64, 64));
    static ImColor curveColor(rgb(255, 255, 200));
    static Vec4 anchorColor(rgb(255, 102, 204));
    static Vec4 tangentColor(rgb(0, 188, 227));
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
        T[i] += GetIO().DeltaTime * 0.2f;
        if (T[i] > 1.0f)
            T[i] = 0.0f;
    }

    bool changed = false;
    ImDrawList *drawList = ImGui::GetWindowDrawList();
    ImVec2 size = ImVec2(600, 600);
    ImGuiWindow *win = ImGui::GetCurrentWindow();
    if (win->SkipItems)
        return changed;

    ImVec2 curveEditorSize = ImVec2(400, 400);

    ImRect winbb(win->DC.CursorPos, win->DC.CursorPos + size);
    if (!ItemAdd(winbb, NULL))
        return changed;

    // draw grid
    auto halfx = (size.x - curveEditorSize.x) / 2;
    auto halfy = (size.y - curveEditorSize.y) / 2;
    ImRect bb(
        ImVec2(winbb.Min.x + halfx, winbb.Min.y + halfy),
        ImVec2(winbb.Max.x - halfx, winbb.Max.y - halfy));
    RenderFrame(bb.Min, bb.Max, gridSquareColor, true, 0.0f);

    float gridStepX = curveEditorSize.x / 4;
    float gridStepY = curveEditorSize.y / 4;

    // horizontal grid lines
    for (int x = -gridStepX; x <= curveEditorSize.x + gridStepX; x += gridStepX)
    {
        drawList->AddLine(
            ImVec2(bb.Min.x + x, bb.Min.y - gridStepY),
            ImVec2(bb.Min.x + x, bb.Max.y + gridStepY),
            gridLineColor);
    }
    // vertical grid lines
    for (int y = -gridStepY; y <= curveEditorSize.y + gridStepY; y += gridStepY)
    {
        drawList->AddLine(
            ImVec2(bb.Min.x - gridStepX, bb.Min.y + y),
            ImVec2(bb.Max.x + gridStepX, bb.Min.y + y),
            gridLineColor);
    }
    // horizontal captions
    for (int x = 0; x <= curveEditorSize.x; x += gridStepX)
    {
        auto pos = screenPosTo01(ImVec2(bb.Min.x + x, bb.Min.y), bb, precision, true);
        drawList->AddText(
            ImVec2(bb.Min.x + x - 5, bb.Min.y - 20),
            gridCaptionColor,
            fmt::format("{}", pos.x).c_str());
    }
    // vertical captions
    for (int y = 0; y <= curveEditorSize.y; y += gridStepY)
    {
        auto pos = screenPosTo01(ImVec2(bb.Min.x, bb.Min.y + y), bb, precision, true);
        drawList->AddText(
            ImVec2(bb.Max.x + 5, bb.Min.y + y - 5),
            gridCaptionColor,
            fmt::format("{}", pos.y).c_str());
    }

    // NOTE: drawing via recursive subdivision
    // paper: https://www.cs.cmu.edu/afs/cs/academic/class/15462-s10/www/lec-slides/lec06.pdf
    Vec2 startEval = curve.EvaluateVec2(0.0);
    Vec2 endEval = curve.EvaluateVec2(1.0);
    drawRecursive(curve, 0.0, 1.0, startEval, endEval, 0.01f, 50, [&](const Vec2 &p0, const Vec2 &p1)
                  {
                      ImVec2 r(p0.x * (bb.Max.x - bb.Min.x) + bb.Min.x, (1-p0.y) * (bb.Max.y - bb.Min.y) + bb.Min.y);
                      ImVec2 s(p1.x * (bb.Max.x - bb.Min.x) + bb.Min.x, (1-p1.y) * (bb.Max.y - bb.Min.y) + bb.Min.y);
                      drawList->AddLine(r, s, curveColor, curveWidth); });

    // ghost anchor that could be added
    ImVec2 mouse = GetIO().MousePos;
    auto mouse01 = screenPosTo01(mouse, bb, precision, true);
    if (mouse01.x >= 0 && mouse01.x <= 1 && !ImGui::IsMouseDragging(0))
    {
        bool allowAdd = true;
        for (const auto &point : curve.Points())
        {
            float distance = Mathf::Distance(Vec2(mouse01.x, mouse01.y), point.P);
            if (distance < 0.05f)
            {
                auto screenPos = screenPosFrom01(ImVec2(point.P.x, point.P.y), bb, true);
                allowAdd = false;
                break;
            }
        }
        if (allowAdd)
        {
            drawList->AddLine(ImVec2(mouse.x, bb.Min.y - gridStepY), ImVec2(mouse.x, bb.Max.y + gridStepY), ImColor(anchorColor));
            auto screenPos = screenPosFrom01(ImVec2(mouse01.x, mouse01.y), bb, true);
            drawList->AddCircle(screenPos, 8, ImColor(anchorColor));
            if (IsMouseClicked(ImGuiMouseButton_Left))
            {
                curve.AddKey(mouse01.x, mouse01.y);
                changed = true;
            }
        }
    }

    // moving animation
    for (int i = 0; i < TN; ++i)
    {
        auto eval = curve.EvaluateVec2(T[i]);
        drawList->AddCircleFilled(ImVec2(eval.x * (bb.Max.x - bb.Min.x) + bb.Min.x, (1 - eval.y) * (bb.Max.y - bb.Min.y) + bb.Min.y), 5, curveColor);
        auto evalF = curve.Evaluate(T[i]);
        ImColor wrongEvalColor = curveColor;
        wrongEvalColor.Value.w = 0.5f;
        drawList->AddCircleFilled(ImVec2(T[i] * (bb.Max.x - bb.Min.x) + bb.Min.x, (1 - evalF) * (bb.Max.y - bb.Min.y) + bb.Min.y), 5, wrongEvalColor);
    }

    // anchors and tangents with lines and text
    const auto points = curve.Points();

    // tangent lines
    for (int i = 0; i < points.size(); ++i)
    {
        const auto &point = points[i];
        const bool isInTangent = i % 3 == 2;
        const bool isOutTangent = i % 3 == 1;
        const float x = bb.Min.x + (point.P.x - curve.StartTime()) / (curve.Time()) * bb.GetWidth();
        const float y = bb.Max.y - (point.P.y * bb.GetHeight());

        if (isOutTangent)
        {
            const Vec2 &prevAnchor = points[i - 1].P;
            auto pos = screenPosFrom01(ImVec2(prevAnchor.x, prevAnchor.y), bb, true);
            drawList->AddLine(ImVec2(x, y), pos, tangentLineColor, tangentWidth);
        }
        if (isInTangent)
        {
            const Vec2 &nextAnchor = points[i + 1].P;
            auto pos = screenPosFrom01(ImVec2(nextAnchor.x, nextAnchor.y), bb, true);
            drawList->AddLine(ImVec2(x, y), pos, tangentLineColor, tangentWidth);
        }
    }
    // anchors and tangents
    Vec2 moved(0, 0);
    Vec2 keyframe;
    int selected = -1;
    int deleted = -1;
    int tangentSplitJoin = -1;
    int affectedAnchor = -1;

    // tangents only
    for (int i = 0; i < points.size(); ++i)
    {
        if (!curve.IsTangent(i))
            continue;

        ImGui::PushID(i);
        const auto &point = points[i];
        const float x = bb.Min.x + (point.P.x - curve.StartTime()) / (curve.Time()) * bb.GetWidth();
        const float y = bb.Max.y - (point.P.y * bb.GetHeight());

        MouseCallback callback;
        callback.OnDoubleClick = [i, &tangentSplitJoin, &deleted]()
        {
            tangentSplitJoin = i;
        };
        callback.OnHover = [&affectedAnchor, i, &curve = std::as_const(curve)]()
        {
            affectedAnchor = curve.IsOutTangent(i) ? i - 1 : i + 1;
        };

        DragHandleStyle dragHandleStyle;
        dragHandleStyle.Color = tangentColor;
        dragHandleStyle.GrabRadius = 4;
        dragHandleStyle.GrabBorder = 1;
        if (DragHandle("AnimationCurveDrag", Vec2(x, y), moved, dragHandleStyle, callback))
        {
            selected = i;
            affectedAnchor = curve.IsOutTangent(i) ? i - 1 : i + 1;
        }
        ImGui::PopID();
    }

    // anchors only
    for (int i = 0; i < points.size(); ++i)
    {
        if (!curve.IsAnchor(i))
            continue;

        ImGui::PushID(i);
        const auto &point = points[i];
        const float x = bb.Min.x + (point.P.x - curve.StartTime()) / (curve.Time()) * bb.GetWidth();
        const float y = bb.Max.y - (point.P.y * bb.GetHeight());

        MouseCallback callback;
        callback.OnHover = [&point = std::as_const(point), drawList, x, y]()
        {
            drawList->AddText(ImVec2(x - 10, y - 25),
                              captionColor,
                              fmt::format("{},{}", point.P.x, point.P.y).c_str());
        };
        callback.OnDoubleClick = [i, &deleted]()
        {
            deleted = i;
        };

        DragHandleStyle dragHandleStyle;
        dragHandleStyle.Color = anchorColor;
        dragHandleStyle.GrabRadius = affectedAnchor == i ? 1 : 5;
        dragHandleStyle.GrabBorder = affectedAnchor == i ? 0 : 2;
        if (DragHandle("AnimationCurveDrag", Vec2(x, y), moved, dragHandleStyle, callback))
        {
            selected = i;
        }
        // debug index
        // drawList->AddText(ImVec2(x - 10, y - 8),
        //                   ImColor(1.0f, 0.0f, 0.0f),
        //                   fmt::format("[{}]", i).c_str());
        ImGui::PopID();
    }
    if (selected != -1)
    {
        auto keyframe = screenPosTo01(moved, bb, precision, true);
        curve.SetPoint(selected, keyframe.x, keyframe.y);
        changed = true;
    }
    if (deleted != -1)
    {
        curve.RemoveKeyframe(deleted / 3);
        changed = true;
    }
    if (tangentSplitJoin != -1)
    {
        curve.ToggleTangentSplitJoin(tangentSplitJoin);
        changed = true;
    }

    return changed;
}

bool UI::DragHandle(const std::string &id, const Vec2 &pos, Vec2 &moved, const UI::DragHandleStyle &style, const MouseCallback &callback)
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

ImVec2 UI::screenPosToMappedRect(const ImVec2 &pos, const ImRect &rect, ImRect &mappedRect) const
{
    float mappedX = (pos.x - rect.Min.x) / (rect.Max.x - rect.Min.x);
    float mappedY = (pos.y - rect.Min.y) / (rect.Max.y - rect.Min.y);

    return ImVec2(
        mappedRect.Min.x + mappedX * (mappedRect.Max.x - mappedRect.Min.x),
        mappedRect.Min.y + mappedY * (mappedRect.Max.y - mappedRect.Min.y));
}

void UI::drawRecursive(const AnimationCurve &curve, float t0, float t1, Vec2 p0, Vec2 p1, float tolerance, int maxSteps, std::function<void(const Vec2 &, const Vec2 &)> drawFn)
{
    Vec2 midPoint = curve.EvaluateVec2((t0 + t1) / 2.0);
    float distance = Mathf::Distance(p1, p0);

    if (distance > tolerance && maxSteps > 0)
    {
        drawRecursive(curve, t0, (t0 + t1) / 2.0, p0, midPoint, tolerance, maxSteps - 1, drawFn);
        drawRecursive(curve, (t0 + t1) / 2.0, t1, midPoint, p1, tolerance, maxSteps - 1, drawFn);
    }
    else
    {
        drawFn(p0, p1);
    }
}
