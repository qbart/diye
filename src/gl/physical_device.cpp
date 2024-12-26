#include "physical_device.hpp"

#include <map>
#include <set>

namespace gl
{
    PhysicalDevice::PhysicalDevice()
    {
    }

    bool PhysicalDevice::IsDiscreteGPU() const
    {
        return properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    }

    bool PhysicalDevice::IsValid() const
    {
        bool valid = handle != VK_NULL_HANDLE;
        bool complete = queueFamilyIndices.graphicsFamily.has_value() && queueFamilyIndices.presentFamily.has_value();
        bool deviceExtensionsSupported = IsExtensionSupported({VK_KHR_SWAPCHAIN_EXTENSION_NAME});
        bool swapChainAdequate = false;
        bool formatAvailable = false;
        if (deviceExtensionsSupported)
        {
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
            for (const auto &availableFormat : swapChainSupport.formats)
            {
                if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                {
                    formatAvailable = true;
                    break;
                }
            }
        }
        return valid && complete && deviceExtensionsSupported && swapChainAdequate && formatAvailable;
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
            vkGetPhysicalDeviceSurfacePresentModesKHR(handle, surface.handle, &presentModeCount, swapChainSupport.presentModes.data());
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
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                queueFamilyIndices.graphicsFamily = familyIndex;

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(
                handle,
                familyIndex,
                surface.handle,
                &presentSupport);

            if (presentSupport)
                queueFamilyIndices.presentFamily = familyIndex;

            //
            ++familyIndex;
        }
    }

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
            devices[i].handle = enumDevices[i];
            devices[i].extensions = GetSupportedPhysicalDeviceExtensions(devices[i].handle);
            vkGetPhysicalDeviceProperties(devices[i].handle, &devices[i].properties);
            vkGetPhysicalDeviceFeatures(devices[i].handle, &devices[i].features);

            devices[i].QuerySwapChainSupport(surface);
            devices[i].QueryQueueFamilies(surface);
        }

        return devices;
    }

    PhysicalDevice SelectBestPhysicalDevice(const std::vector<PhysicalDevice> &devices)
    {
        std::multimap<int, PhysicalDevice> candidates;

        for (const auto &device : devices)
        {
            int score = device.properties.limits.maxImageDimension2D;

            if (device.IsDiscreteGPU())
                score += 1000;

            candidates.insert(std::make_pair(score, device));
        }
        PhysicalDevice best;
        int score = 0;
        for (const auto &c : candidates)
        {
            if (c.first > score)
            {
                score = c.first;
                best = c.second;
            }
        }

        fmtx::Info(fmt::format("Selected device: {}", best.properties.deviceName));
        fmtx::Info(fmt::format("Selected device API version: {}", best.properties.apiVersion));
        fmtx::Info(fmt::format("Selected device driver version: {}", best.properties.driverVersion));

        return best;
    }

}