#pragma once

#include "core.hpp"
#include "surface.hpp"

namespace gl
{
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
    };

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    class PhysicalDevice
    {
    public:
        VkPhysicalDevice handle;
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceFeatures features;
        std::vector<VkExtensionProperties> extensions;
        std::vector<VkQueueFamilyProperties> queueFamilies;
        QueueFamilyIndices queueFamilyIndices;
        SwapChainSupportDetails swapChainSupport;

        PhysicalDevice();

        bool IsDiscreteGPU() const;
        bool IsValid() const;
        bool IsExtensionSupported(const gl::CStrings &extensions) const;
        void QuerySwapChainSupport(const Surface &surface);
        void QueryQueueFamilies(const Surface &surface);
    };

    std::vector<VkExtensionProperties> GetSupportedPhysicalDeviceExtensions(const VkPhysicalDevice &device);
    std::vector<PhysicalDevice> GetPhysicalDevices(const gl::Instance &instance, const gl::Surface &surface);
    PhysicalDevice SelectBestPhysicalDevice(const std::vector<PhysicalDevice> &devices);
}