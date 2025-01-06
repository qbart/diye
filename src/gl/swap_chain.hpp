#pragma once

#include "core.hpp"
#include "physical_device.hpp"
#include "surface.hpp"
#include "device.hpp"

namespace gl
{
    class Image;

    class SwapChain
    {
    public:
        VkSwapchainKHR handle;
        std::vector<Image> images;
        VkFormat imageFormat;
        VkExtent2D extent;

        SwapChain();

        bool Create(const Device &device, const Surface &surface, const PhysicalDevice &physicalDevice);
        void Destroy(const Device &device);
        VkResult AcquireNextImage(const Device &device, uint32_t *imageIndex, VkSemaphore semaphore, VkFence fence = VK_NULL_HANDLE) const;
    };
}