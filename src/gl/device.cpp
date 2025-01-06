#include "device.hpp"

#include <set>
#include <vector>
#include "queue.hpp"

namespace gl
{
    Device::Device() : handle(VK_NULL_HANDLE),
                       graphicsQueue(VK_NULL_HANDLE),
                       presentQueue(VK_NULL_HANDLE),
                       createInfo({})
    {
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.enabledExtensionCount = 0;
        createInfo.enabledLayerCount = 0;

#ifdef __APPLE__
        requiredExtensions.emplace_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

        createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();
    }

    bool Device::Create(const PhysicalDevice &physicalDevice)
    {
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {
            physicalDevice.queueFamilyIndices.graphicsFamily.value(),
            physicalDevice.queueFamilyIndices.presentFamily.value()};
        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;

        if (vkCreateDevice(physicalDevice.handle, &createInfo, nullptr, &handle) != VK_SUCCESS)
        {
            fmtx::Error("Failed to create logical device");
            return false;
        }

        vkGetDeviceQueue(handle, physicalDevice.queueFamilyIndices.graphicsFamily.value(), 0, &graphicsQueue.handle);
        vkGetDeviceQueue(handle, physicalDevice.queueFamilyIndices.presentFamily.value(), 0, &presentQueue.handle);

        fmtx::Info("Logical device created");

        return true;
    }

    void Device::Destroy()
    {
        if (handle != VK_NULL_HANDLE)
            vkDestroyDevice(handle, nullptr);
    }

    VkResult Device::WaitIdle() const
    {
        return vkDeviceWaitIdle(handle);
    }

    void Device::RequireSwapchainExtension()
    {
        requiredExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();
    }

    void Device::SetRequiredExtensions(const CStrings &extensions)
    {
        for (const auto &ext : extensions)
            requiredExtensions.emplace_back(ext);

        createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();
    }

    void Device::EnableValidationLayers()
    {
        validationLayers = CStrings({"VK_LAYER_KHRONOS_validation"});
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }

    void Device::UpdateDescriptorSets(const std::vector<VkWriteDescriptorSet> &descriptorWrites)
    {
        vkUpdateDescriptorSets(handle, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}