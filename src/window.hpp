#pragma once
#include "deps/sdl.hpp"
#include "deps/vulkan.hpp"
#include "input.hpp"

class Window
{
public:
    using Ptr = std::shared_ptr<Window>;

    static Ptr New(int w, int h, const std::string &title);
    Window() {}
    ~Window();

    SDL_Window *Get() const { return wnd; }
    void Swap();
    void PollEvents();
    void Close();
    void Debug();
    inline bool IsOpen() const { return isOpen; }
    inline const Dimension &Size() const { return size; }
    inline Input GetInput() { return Input(inputs); }
    inline Vec2 MousePosition() const { return mousePos; }
    inline Vec2 MouseRelativePosition() const { return mouseRelPos; }
    inline Vec2 MouseWheel() const { return mouseWheel; }
    inline bool MouseButtonDown(uint8 button) const { return HashMapHasKey(mouseInputs, button) && mouseInputs.at(button); }
    inline bool MouseWheelScrolled() const { return SDL_GetTicks() - lastTimeWheeled < 100; }
    inline bool WasResized() const { return resized; }

private:
    bool isOpen = false;
    Dimension size;
    SDL_Window *wnd = nullptr;
    SDL_Event event;
    HashMap<int32, bool> inputs;
    HashMap<uint8, bool> mouseInputs;
    Vec2 mousePos = Vec2(0, 0);
    Vec2 mouseRelPos = Vec2(0, 0);
    Vec2 mouseWheel = Vec2(0, 0);
    uint64 lastTimeWheeled = 0;
    bool resized = false;

    vulkan::Instance instance;
};