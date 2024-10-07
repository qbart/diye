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

void UI::BeginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UI::EndFrame()
{
    ImGui::Render();
}

void UI::Draw()
{
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
