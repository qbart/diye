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

void UI::TranslateGizmo(const Camera &camera, Transform &transform)
{
    auto mat = transform.GetModelMatrix();
    auto skew = Vec3(0.0f);
    auto perspective = Vec4(0.0f);
    auto rotation = transform.rotation;
    auto scale = transform.scale;
    ImGuizmo::Manipulate(
        glm::value_ptr(camera.GetViewMatrix()),
        glm::value_ptr(camera.GetProjection()),
        ImGuizmo::OPERATION::TRANSLATE,
        ImGuizmo::MODE::WORLD,
        glm::value_ptr(mat));
    glm::decompose(mat, scale, rotation, transform.position, skew, perspective);
    transform.Update();
}

void UI::RotationGizmo(const Camera &camera, Transform &transform)
{
    auto mat = transform.GetModelMatrix();
    auto skew = Vec3(0.0f);
    auto perspective = Vec4(0.0f);
    auto position = transform.position;
    auto rotation = transform.rotation;
    auto scale = transform.scale;
    ImGuizmo::Manipulate(
        glm::value_ptr(camera.GetViewMatrix()),
        glm::value_ptr(camera.GetProjection()),
        ImGuizmo::OPERATION::ROTATE,
        ImGuizmo::MODE::LOCAL,
        glm::value_ptr(mat));
    glm::decompose(mat, scale, transform.rotation, position, skew, perspective);
    transform.Update();
}

void UI::ScaleGizmo(const Camera &camera, Transform &transform)
{
    auto mat = transform.GetModelMatrix();
    auto skew = Vec3(0.0f);
    auto perspective = Vec4(0.0f);
    auto rotation = transform.rotation;
    auto position = transform.position;
    ImGuizmo::Manipulate(
        glm::value_ptr(camera.GetViewMatrix()),
        glm::value_ptr(camera.GetProjection()),
        ImGuizmo::OPERATION::SCALE,
        ImGuizmo::MODE::WORLD,
        glm::value_ptr(mat));
    glm::decompose(mat, transform.scale, rotation, position, skew, perspective);
    transform.Update();
}
