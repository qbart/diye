#pragma once

#define SDL_MAIN_HANDLED

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <string>
#include <fmt/format.h>
#include <vector>
#include "../core/all.hpp"
#include <unordered_map>

namespace sdl
{
    std::string GetError();
    int Init();
    void Quit();
    SDL_Window *CreateWindow(const std::string &title, int w, int h);
    void DestroyWindow(SDL_Window *wnd);

    std::vector<const char *> GetVulkanExtensions(SDL_Window *wnd, bool debug = false);
    bool CreateVulkanSurface(SDL_Window *wnd, VkInstance instance, VkSurfaceKHR *surface);
    VkExtent2D GetVulkanFramebufferSize(SDL_Window *window);
};

namespace sdl
{
    struct Ticks
    {
    public:
        inline void Update()
        {
            ticks = SDL_GetTicks64();
            last = now;
            now = ticks;
            dt = (now - last) / 1000.0;
        }
        inline double DeltaTime()
        {
            return dt;
        }
        inline double TotalMilliseconds()
        {
            return ticks / 1000.0;
        }

    private:
        double dt = 0;
        double last = 0;
        double now = 0;
        double ticks = 0;
    };
};

namespace sdl
{
    class Window
    {
    public:
        Window();
        ~Window();
        bool Init(int w, int h, const std::string &title);
        void Shutdown();

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
        void FreeCameraControls(Camera &camera, float dt);

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

}