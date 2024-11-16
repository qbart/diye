#pragma once
#include "deps/sdl.hpp"
#include "core/all.hpp"

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
    inline Vec2 MousePosition() const { return mousePos; }
    inline Vec2 MouseRelativePosition() const { return mouseRelPos; }
    inline Vec2 MouseWheel() const { return mouseWheel; }
    inline bool MouseButtonDown(uint8 button) const { return HashMapHasKey(mouseInputs, button) && mouseInputs.at(button); }
    inline bool MouseWheelScrolled() const { return SDL_GetTicks() - lastTimeWheeled < 100; }
    inline bool WasResized() const { return resized; }
    bool KeyJustReleased(int key);
    bool KeyDown(int key);

private:
    bool isOpen = false;
    Dimension size;
    SDL_Window *wnd = nullptr;
    SDL_Event event;
    SDL_GLContext glContext = nullptr;
    HashMap<uint8, bool> mouseInputs;
    HashMap<int32, bool> wasDown;
    HashMap<int32, bool> inputs;
    Vec2 mousePos = Vec2(0, 0);
    Vec2 mouseRelPos = Vec2(0, 0);
    Vec2 mouseWheel = Vec2(0, 0);
    uint64 lastTimeWheeled = 0;
    bool resized = false;
};