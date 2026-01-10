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
    VkFormatProperties formatProperties;
    std::vector<VkExtensionProperties> extensions;
    std::vector<VkQueueFamilyProperties> queueFamilies;
    QueueFamilyIndices queueFamilyIndices;
    SwapChainSupportDetails swapChainSupport;
    VkPhysicalDeviceMemoryProperties memProperties;
    VkFormat depthFormat;

    PhysicalDevice();

    bool IsDiscreteGPU() const;
    bool IsValid() const;
    bool IsExtensionSupported(const gl::CStrings &extensions) const;
    void QuerySwapChainSupport(const Surface &surface);
    void QueryQueueFamilies(const Surface &surface);
    int FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
    VkFilter TrySampledImageFilterLinear(VkFormat format) const;
    std::vector<VkSampleCountFlagBits> GetSupportedSampleCounts() const;
    VkSampleCountFlagBits GetMaxUsableSampleCount() const;
};

std::vector<VkExtensionProperties> GetSupportedPhysicalDeviceExtensions(const VkPhysicalDevice &device);
std::vector<PhysicalDevice> GetPhysicalDevices(const gl::Instance &instance, const gl::Surface &surface);
PhysicalDevice SelectBestPhysicalDevice(const std::vector<PhysicalDevice> &devices);
} // namespace gl