#include "surface.hpp"

gl::Surface::Surface() : handle(VK_NULL_HANDLE),
                         window(nullptr)
{
}

bool gl::Surface::Create(const Instance &instance, SDL_Window *window)
{
    this->window = window;

    if (!sdl::CreateVulkanSurface(window, instance.handle, &handle))
    {
        fmtx::Error("Failed to create Vulkan surface");
        return false;
    }
    fmtx::Info("Vulkan surface created");

    return true;
}

void gl::Surface::Destroy(const Instance &instance)
{
    if (handle != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(instance.handle, handle, nullptr);
}
