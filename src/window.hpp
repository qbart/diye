#pragma once
#include "deps/sdl.hpp"
#include "input.hpp"

class Window
{
public:
    static std::unique_ptr<Window> New(int w, int h, const std::string &title);
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

private:
    bool isOpen = false;
    Dimension size;
    SDL_Window *wnd = nullptr;
    SDL_Event event;
    SDL_GLContext glContext = nullptr;
    std::unordered_map<int32, bool> inputs;
};