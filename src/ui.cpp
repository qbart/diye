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
    auto mat = transform.ModelMatrix(Transform::Space::World);
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
    bool changed = false;
    static const int resolution = 64;
    static const int CURVE_WIDTH = 4;
    static const int GRAB_RADIUS = 8;
    static const int GRAB_BORDER = 2;
    static const int LINE_WIDTH = 1;
    Vec2 values[resolution + 1];
    ImDrawList *drawList = ImGui::GetWindowDrawList();
    ImVec2 size = ImVec2(600, 600);
    ImGuiWindow *win = ImGui::GetCurrentWindow();
    if (win->SkipItems)
        return changed;

    ImVec2 curveEditorSize = ImVec2(400, 400);

    ImRect winbb(win->DC.CursorPos, win->DC.CursorPos + size);
    if (!ItemAdd(winbb, NULL))
        return changed;

    auto halfx = (size.x - curveEditorSize.x) / 2;
    auto halfy = (size.y - curveEditorSize.y) / 2;
    ImRect bb(
        ImVec2(winbb.Min.x + halfx, winbb.Min.y + halfy),
        ImVec2(winbb.Max.x - halfx, winbb.Max.y - halfy));

    RenderFrame(bb.Min, bb.Max, GetColorU32(ImGuiCol_TabActive), true, 0.0f);

    for (int i = 0; i <= curveEditorSize.x; i += (curveEditorSize.x / 4))
    {
        drawList->AddLine(
            ImVec2(bb.Min.x + i, bb.Min.y),
            ImVec2(bb.Min.x + i, bb.Max.y),
            GetColorU32(ImGuiCol_TextDisabled));
    }
    for (int i = 0; i <= curveEditorSize.y; i += (curveEditorSize.y / 4))
    {
        drawList->AddLine(
            ImVec2(bb.Min.x, bb.Min.y + i),
            ImVec2(bb.Max.x, bb.Min.y + i),
            GetColorU32(ImGuiCol_TextDisabled));
    }

    float timeStep = curve.Time() / (resolution - 1);
    for (int i = 0; i <= resolution; ++i)
    {
        float t = 0 + i * timeStep;
        values[i].x = t;
        values[i].y = curve.Evaluate(t);
    }
    ImColor color(0.0f, 1.0f, 0.0f);

    for (int i = 0; i < resolution; ++i)
    {
        ImVec2 p = {values[i + 0].x, 1 - values[i + 0].y};
        ImVec2 q = {values[i + 1].x, 1 - values[i + 1].y};
        ImVec2 r(p.x * (bb.Max.x - bb.Min.x) + bb.Min.x, p.y * (bb.Max.y - bb.Min.y) + bb.Min.y);
        ImVec2 s(q.x * (bb.Max.x - bb.Min.x) + bb.Min.x, q.y * (bb.Max.y - bb.Min.y) + bb.Min.y);
        drawList->AddLine(r, s, color, CURVE_WIDTH);
    }
    ImVec2 mouse = GetIO().MousePos;
    auto mouse01 = screenPosTo01(mouse, bb, 3, true);
    if (mouse01.x >= 0 && mouse01.x <= 1)
    {
        float ghostY = curve.Evaluate(mouse01.x);
        float distance = (ghostY - mouse01.y) * (ghostY - mouse01.y);
        if (distance < 0.02f)
        {
            auto screenPos = screenPosFrom01(ImVec2(mouse01.x, ghostY), bb, true);
            drawList->AddCircleFilled(screenPos, 8, color);
            if (IsMouseClicked(0))
            {
                curve.AddKey(mouse01.x, mouse01.y);
                changed = true;
            }
        }
    }

    // {
    //     // handle grabbers
    //     ImVec2 mouse = GetIO().MousePos, pos[2];
    //     float distance[2];

    //     for (int i = 0; i < curve.Keyframes().size() - 1; ++i)
    //     {
    //         const auto &kf = curve.Keyframes()[i];
    //         const auto &next = curve.Keyframes()[i + 1];
    //         pos[i] = ImVec2(kf.time, 1 - next.time) * (bb.Max - bb.Min) + bb.Min;
    //         distance[i] = (pos[i].x - mouse.x) * (pos[i].x - mouse.x) + (pos[i].y - mouse.y) * (pos[i].y - mouse.y);
    //     }

    //     int selected = distance[0] < distance[1] ? 0 : 1;
    //     // if (distance[selected] < (4 * GRAB_RADIUS * 4 * GRAB_RADIUS))
    //     // {
    //     //     SetTooltip("(%4.3f, %4.3f)", P[selected * 2 + 0], P[selected * 2 + 1]);

    //     //     if (/*hovered &&*/ (IsMouseClicked(0) || IsMouseDragging(0)))
    //     //     {
    //     //         float &px = (P[selected * 2 + 0] += GetIO().MouseDelta.x / Canvas.x);
    //     //         float &py = (P[selected * 2 + 1] -= GetIO().MouseDelta.y / Canvas.y);

    //     //         if (true)
    //     //         {
    //     //             px = (px < 0 ? 0 : (px > 1 ? 1 : px));
    //     //             py = (py < 0 ? 0 : (py > 1 ? 1 : py));
    //     //         }

    //     //         changed = true;
    //     //     }
    //     // }
    // }

    // draw lines and grabbers
    const auto points = curve.Points();

    ImVec2 moved(0, 0);
    Vec2 keyframe;
    int selected = -1;
    for (int i = 0; i < points.size(); ++i)
    {
        const auto &point = points[i];
        bool isAnchor = i % 3 == 0;
        bool isInTangent = i % 3 == 2;
        bool isOutTangent = i % 3 == 1;
        float x = bb.Min.x + (point.x - points.front().x) / (curve.Time()) * bb.GetWidth();
        float y = bb.Max.y - (point.y * bb.GetHeight());
        // Draw keyframe point

        if (isOutTangent)
        {
            const Vec2 &outTangent = points[i - 1];
            float otx = bb.Min.x + (outTangent.x - points.front().x) / (curve.Time()) * bb.GetWidth();
            float oty = bb.Max.y - (outTangent.y * bb.GetHeight());

            drawList->AddLine(ImVec2(x, y), ImVec2(otx, oty), ImColor(0, 128, 0), 2.0f);
        }
        if (isInTangent)
        {
            const Vec2 &inTangent = points[i + 1];
            float itx = bb.Min.x + (inTangent.x - points.front().x) / (curve.Time()) * bb.GetWidth();
            float ity = bb.Max.y - (inTangent.y * bb.GetHeight());

            drawList->AddLine(ImVec2(x, y), ImVec2(itx, ity), ImColor(0, 128, 0), 2.0f);
        }

        drawList->AddText(ImVec2(x - 10, y + 10), ImColor(1.0f, 1.0f, 1.0f), fmt::format("{},{}", point.x, point.y).c_str());
        if (DragHandle("AnimationCurveDrag" + i, ImVec2(x, y), moved, isAnchor ? rgb(0, 188, 227) : rgb(255, 102, 204)))
        {
            selected = i;
            changed = true;
        }
    }
    if (selected != -1)
    {
        auto keyframe = screenPosTo01(moved, bb, 3, true);
        curve.SetKeyframe(selected, keyframe.x, keyframe.y);
    }

    return changed;
}

bool UI::DragHandle(const std::string &id, const ImVec2 &pos, ImVec2 &moved, const Vec3 &color)
{
    bool changed = false;
    static const int GRAB_RADIUS = 8;
    static const int GRAB_BORDER = 2;
    ImVec2 handlePos = ImVec2(pos.x, pos.y);
    ImDrawList *drawList = ImGui::GetWindowDrawList();

    auto buttonPos = ImVec2(
        handlePos.x - 5.0f,
        handlePos.y - 5.0f);

    auto cursor = ImGui::GetCursorScreenPos();
    ImGui::SetCursorScreenPos(buttonPos);
    ImGui::InvisibleButton(("DragHandle__" + id).c_str(), ImVec2(10, 10));
    ImGui::SetCursorScreenPos(cursor);
    float alpha = ImGui::IsItemActive() || ImGui::IsItemHovered() ? 0.5f : 1.0f;
    drawList->AddCircleFilled(handlePos, GRAB_RADIUS, ImColor(1.0f, 1.0f, 1.0f));
    drawList->AddCircleFilled(handlePos, GRAB_RADIUS - GRAB_BORDER, ImColor(color.x, color.y, color.z, alpha));
    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
    {
        cursor = ImGui::GetIO().MouseDelta;
        moved = ImVec2(handlePos.x + cursor.x, handlePos.y + cursor.y);
        changed = true;
    }
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
    return ImVec2(pos.x * (rect.Max.x - rect.Min.x) + rect.Min.x,
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
