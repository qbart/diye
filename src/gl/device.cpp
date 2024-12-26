#include "device.hpp"

#include <set>
#include "render_pass.hpp"

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
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;

        if (vkCreateDevice(physicalDevice.handle, &createInfo, nullptr, &handle) != VK_SUCCESS)
        {
            fmtx::Error("Failed to create logical device");
            return false;
        }

        vkGetDeviceQueue(handle, physicalDevice.queueFamilyIndices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(handle, physicalDevice.queueFamilyIndices.presentFamily.value(), 0, &presentQueue);

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

    std::vector<VkImageView> Device::CreateImageViews(VkFormat format, const std::vector<VkImage> &images)
    {
        std::vector<VkImageView> views;
        std::vector<bool> valid;
        views.resize(images.size());
        valid.resize(images.size());
        bool allValid = true;

        for (size_t i = 0; i < images.size(); i++)
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = images[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = format;

            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            valid[i] = vkCreateImageView(handle, &createInfo, nullptr, &views[i]) == VK_SUCCESS;
            if (!valid[i])
                allValid = false;
        }

        if (!allValid)
        {
            for (size_t i = 0; i < views.size(); i++)
            {
                if (valid[i])
                    vkDestroyImageView(handle, views[i], nullptr);
            }
            views.clear();
        }

        return views;
    }

    void Device::DestroyImageViews(const std::vector<VkImageView> &views)
    {
        for (auto imageView : views)
        {
            vkDestroyImageView(handle, imageView, nullptr);
        }
    }

    std::vector<VkFramebuffer> Device::CreateFramebuffers(const RenderPass &renderPass, const std::vector<VkImageView> &views, const VkExtent2D &extent)
    {
        std::vector<VkFramebuffer> framebuffers;
        std::vector<bool> valid;
        framebuffers.resize(views.size());
        valid.resize(views.size());
        bool allValid = true;

        for (size_t i = 0; i < views.size(); i++)
        {
            VkImageView attachments[] = {views[i]};

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass.handle;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = extent.width;
            framebufferInfo.height = extent.height;
            framebufferInfo.layers = 1;

            valid[i] = vkCreateFramebuffer(handle, &framebufferInfo, nullptr, &framebuffers[i]) == VK_SUCCESS;
            if (!valid[i])
                allValid = false;
        }

        if (!allValid)
        {
            for (size_t i = 0; i < framebuffers.size(); i++)
            {
                if (valid[i])
                    vkDestroyFramebuffer(handle, framebuffers[i], nullptr);
            }
            framebuffers.clear();
        }

        return framebuffers;
    }

    void Device::DestroyFramebuffers(const std::vector<VkFramebuffer> &framebuffers)
    {
        for (auto fb : framebuffers)
        {
            vkDestroyFramebuffer(handle, fb, nullptr);
        }
    }
}