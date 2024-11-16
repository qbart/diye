#pragma once

#include "deps/sdl.hpp"
#include "core/all.hpp"
#include <unordered_map>

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
    Vec2 MousePosition() const { return mousePos; }
    Vec2 MouseRelativePosition() const { return mouseRelPos; }
    Vec2 MouseWheel() const { return mouseWheel; }
    bool MouseButtonDown(uint8 button) const;
    bool MouseButtonUp(uint8 button);
    bool MouseWheelScrolled() const { return SDL_GetTicks64() - lastTimeWheeled < 100; }
    bool KeyJustReleased(int key);
    bool KeyDown(int key);
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
    HashMap<int32, bool> wasDown;
    HashMap<uint8, bool> mouseInputs;
    HashMap<uint8, bool> mouseWasDown;
    Vec2 mousePos;
    Vec2 mouseRelPos;
    Vec2 mouseWheel;
    std::uint64_t lastTimeWheeled;
};
