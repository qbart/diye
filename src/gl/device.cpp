#include "device.hpp"

#include "queue.hpp"
#include <set>
#include <vector>

namespace gl
{
Device::Device() :
    handle(VK_NULL_HANDLE),
    deviceFeatures({}),
    graphicsQueue(VK_NULL_HANDLE),
    presentQueue(VK_NULL_HANDLE),
    createInfo({}),
    DynamicRenderingEnabled(false)
{
    createInfo.sType                 = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext                 = nullptr;
    createInfo.enabledExtensionCount = 0;
    createInfo.enabledLayerCount     = 0;

    deviceFeatures.samplerAnisotropy = VK_TRUE;

#ifdef __APPLE__
    requiredExtensions.emplace_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

    createInfo.enabledExtensionCount   = static_cast<uint32_t>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();
}

bool Device::Create(const PhysicalDevice &physicalDevice)
{
    for (const auto &ext : requiredExtensions) fmtx::Debug(fmt::format("Require extension: {}", ext));

    for (const auto &layer : validationLayers) fmtx::Debug(fmt::format("Validation layer: {}", layer));

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        physicalDevice.queueFamilyIndices.graphicsFamily.value(),
        physicalDevice.queueFamilyIndices.presentFamily.value()
    };
    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount       = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    if (DynamicRenderingEnabled)
    {
        VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeature{};
        dynamicRenderingFeature.sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
        dynamicRenderingFeature.dynamicRendering = VK_TRUE;
        createInfo.pNext                         = &dynamicRenderingFeature;
    }

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos    = queueCreateInfos.data();
    createInfo.pEnabledFeatures     = &deviceFeatures;

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
    if (handle != VK_NULL_HANDLE) vkDestroyDevice(handle, nullptr);
}

VkResult Device::WaitIdle() const { return vkDeviceWaitIdle(handle); }

VkResult Device::WaitForFences(uint32_t count, const VkFence *pFences) const
{
    return vkWaitForFences(handle, count, pFences, VK_TRUE, UINT64_MAX);
}

void Device::RequireSwapchainExtension()
{
    requiredExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    createInfo.enabledExtensionCount   = static_cast<uint32_t>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();
}

void Device::RequireDynamicRendering()
{
    requiredExtensions.emplace_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    createInfo.enabledExtensionCount   = static_cast<uint32_t>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();
    DynamicRenderingEnabled            = true;
}

void Device::SetRequiredExtensions(const CStrings &extensions)
{
    for (const auto &ext : extensions) requiredExtensions.emplace_back(ext);

    createInfo.enabledExtensionCount   = static_cast<uint32_t>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();
}

void Device::EnableValidationLayers()
{
    validationLayers               = CStrings({"VK_LAYER_KHRONOS_validation"});
    createInfo.enabledLayerCount   = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
}

void Device::UpdateDescriptorSets(const std::vector<VkWriteDescriptorSet> &descriptorWrites)
{
    vkUpdateDescriptorSets(handle, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void Device::EnableSampleRateShading() { deviceFeatures.sampleRateShading = VK_TRUE; }
} // namespace gl
