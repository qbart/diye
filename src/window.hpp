#pragma once

#include "deps/sdl.hpp"
#include "input.hpp"

class Window
{
public:
    Window();
    ~Window();
    bool Init(int w, int h, const std::string &title);

    SDL_Window *Get() const { return wnd; }
    void PollEvents();
    void Close();
    bool IsOpen() const { return isOpen; }
    const Dimension &Size() const { return size; }
    Input GetInput() { return Input(inputs); }
    Vec2 MousePosition() const { return mousePos; }
    Vec2 MouseRelativePosition() const { return mouseRelPos; }
    Vec2 MouseWheel() const { return mouseWheel; }
    bool MouseButtonDown(uint8 button) const { return HashMapHasKey(mouseInputs, button) && mouseInputs.at(button); }
    bool MouseWheelScrolled() const { return SDL_GetTicks64() - lastTimeWheeled < 100; }
    bool WasResized() const { return resized; }
    bool IsActive() const { return active; }
    void SetResized(bool r) { resized = r; }

private:
    bool active;
    bool isOpen;
    bool resized;

    Dimension size;
    SDL_Window *wnd;

    SDL_Event event;
    HashMap<int32, bool> inputs;
    HashMap<uint8, bool> mouseInputs;
    Vec2 mousePos;
    Vec2 mouseRelPos;
    Vec2 mouseWheel;
    std::uint64_t lastTimeWheeled;
};
