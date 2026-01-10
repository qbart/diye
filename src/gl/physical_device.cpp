#include "physical_device.hpp"

#include <map>
#include <set>

namespace gl
{
PhysicalDevice::PhysicalDevice() : handle(VK_NULL_HANDLE), depthFormat(VK_FORMAT_UNDEFINED) {}

bool PhysicalDevice::IsDiscreteGPU() const { return properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU; }

bool PhysicalDevice::IsValid() const
{
    bool valid    = handle != VK_NULL_HANDLE;
    bool complete = queueFamilyIndices.graphicsFamily.has_value() && queueFamilyIndices.presentFamily.has_value();
    bool deviceExtensionsSupported = IsExtensionSupported({VK_KHR_SWAPCHAIN_EXTENSION_NAME});
    bool swapChainAdequate         = false;
    bool formatAvailable           = false;
    if (deviceExtensionsSupported)
    {
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        for (const auto &availableFormat : swapChainSupport.formats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                formatAvailable = true;
                break;
            }
        }
    }
    return valid && complete && deviceExtensionsSupported && swapChainAdequate && formatAvailable &&
           features.samplerAnisotropy;
}

bool PhysicalDevice::IsExtensionSupported(const gl::CStrings &checkExtensions) const
{
    std::set<std::string> requiredExtensions(checkExtensions.begin(), checkExtensions.end());

    for (const auto &ext : extensions)
    {
        requiredExtensions.erase(ext.extensionName);
    }

    return requiredExtensions.empty();
}

void PhysicalDevice::QuerySwapChainSupport(const gl::Surface &surface)
{
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(handle, surface.handle, &swapChainSupport.capabilities);
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(handle, surface.handle, &formatCount, nullptr);
    if (formatCount != 0)
    {
        swapChainSupport.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(handle, surface.handle, &formatCount, swapChainSupport.formats.data());
    }
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(handle, surface.handle, &presentModeCount, nullptr);
    if (presentModeCount != 0)
    {
        swapChainSupport.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            handle, surface.handle, &presentModeCount, swapChainSupport.presentModes.data()
        );
    }
}

void PhysicalDevice::QueryQueueFamilies(const gl::Surface &surface)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(handle, &queueFamilyCount, nullptr);
    queueFamilies.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(handle, &queueFamilyCount, queueFamilies.data());

    int familyIndex = 0;
    for (const auto &queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) queueFamilyIndices.graphicsFamily = familyIndex;

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(handle, familyIndex, surface.handle, &presentSupport);

        if (presentSupport) queueFamilyIndices.presentFamily = familyIndex;

        //
        ++familyIndex;
    }
}

int PhysicalDevice::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const
{
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }
    return -1;
}

VkFilter PhysicalDevice::TrySampledImageFilterLinear(VkFormat format) const
{
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(handle, format, &formatProperties);
    return formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT
               ? VK_FILTER_LINEAR
               : VK_FILTER_NEAREST;
}

std::vector<VkSampleCountFlagBits> PhysicalDevice::GetSupportedSampleCounts() const
{
    auto result               = std::vector<VkSampleCountFlagBits>();
    VkSampleCountFlags counts = properties.limits.framebufferDepthSampleCounts &
                                properties.limits.framebufferColorSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT) result.push_back(VK_SAMPLE_COUNT_64_BIT);

    if (counts & VK_SAMPLE_COUNT_32_BIT) result.push_back(VK_SAMPLE_COUNT_32_BIT);

    if (counts & VK_SAMPLE_COUNT_16_BIT) result.push_back(VK_SAMPLE_COUNT_16_BIT);

    if (counts & VK_SAMPLE_COUNT_8_BIT) result.push_back(VK_SAMPLE_COUNT_8_BIT);

    if (counts & VK_SAMPLE_COUNT_4_BIT) result.push_back(VK_SAMPLE_COUNT_4_BIT);

    if (counts & VK_SAMPLE_COUNT_2_BIT) result.push_back(VK_SAMPLE_COUNT_2_BIT);

    result.push_back(VK_SAMPLE_COUNT_1_BIT);

    return result;
}

VkSampleCountFlagBits PhysicalDevice::GetMaxUsableSampleCount() const { return GetSupportedSampleCounts().front(); }

std::vector<VkExtensionProperties> GetSupportedPhysicalDeviceExtensions(const VkPhysicalDevice &device)
{
    uint32_t count = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> extensions(count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, extensions.data());
    return extensions;
}

std::vector<PhysicalDevice> GetPhysicalDevices(const gl::Instance &instance, const gl::Surface &surface)
{
    std::vector<VkPhysicalDevice> enumDevices;
    uint32_t count;
    vkEnumeratePhysicalDevices(instance.handle, &count, nullptr);

    enumDevices.resize(count);
    vkEnumeratePhysicalDevices(instance.handle, &count, enumDevices.data());

    std::vector<PhysicalDevice> devices(enumDevices.size());
    for (int i = 0; i < devices.size(); ++i)
    {
        // device
        devices[i].handle     = enumDevices[i];
        devices[i].extensions = GetSupportedPhysicalDeviceExtensions(devices[i].handle);
        vkGetPhysicalDeviceProperties(devices[i].handle, &devices[i].properties);
        vkGetPhysicalDeviceFeatures(devices[i].handle, &devices[i].features);

        devices[i].QuerySwapChainSupport(surface);
        devices[i].QueryQueueFamilies(surface);
        vkGetPhysicalDeviceMemoryProperties(devices[i].handle, &devices[i].memProperties);

        // take the best depth format with stencil buffer
        VkFormatProperties depthFormatPropertiesD32S8;
        VkFormatProperties depthFormatPropertiesD24S8;
        vkGetPhysicalDeviceFormatProperties(
            devices[i].handle, VK_FORMAT_D32_SFLOAT_S8_UINT, &depthFormatPropertiesD32S8
        );
        vkGetPhysicalDeviceFormatProperties(
            devices[i].handle, VK_FORMAT_D24_UNORM_S8_UINT, &depthFormatPropertiesD24S8
        );

        if (depthFormatPropertiesD32S8.optimalTilingFeatures &
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT == VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            devices[i].depthFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
        else if (depthFormatPropertiesD24S8.optimalTilingFeatures &
                 VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT == VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            devices[i].depthFormat = VK_FORMAT_D24_UNORM_S8_UINT;
        else
        {
            fmtx::Error("Failed to find depth format with stencil buffer");
        }
    }

    return devices;
}

PhysicalDevice SelectBestPhysicalDevice(const std::vector<PhysicalDevice> &devices)
{
    std::multimap<int, PhysicalDevice> candidates;

    for (const auto &device : devices)
    {
        int score = device.properties.limits.maxImageDimension2D;

        if (device.IsDiscreteGPU()) score += 1000;

        candidates.insert(std::make_pair(score, device));
    }
    PhysicalDevice best;
    int score = 0;
    for (const auto &c : candidates)
    {
        if (c.first > score)
        {
            score = c.first;
            best  = c.second;
        }
    }

    fmtx::Info(fmt::format("Selected device: {}", best.properties.deviceName));
    fmtx::Info(fmt::format("Selected device API version: {}", best.properties.apiVersion));
    fmtx::Info(fmt::format("Selected device driver version: {}", best.properties.driverVersion));

    return best;
}

} // namespace gl