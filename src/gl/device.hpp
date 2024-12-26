#pragma once

#include "core.hpp"
#include "physical_device.hpp"

namespace gl
{
    class Device
    {
    public:
        VkDeviceCreateInfo createInfo;
        VkDevice handle;
        VkQueue graphicsQueue;
        VkQueue presentQueue;
        std::vector<const char *> requiredExtensions;
        std::vector<const char *> validationLayers;

        Device();
        bool Create(const PhysicalDevice &physicalDevice);
        void Destroy();
        VkResult WaitIdle() const;
        void RequireSwapchainExtension();
        void SetRequiredExtensions(const CStrings &extensions);
        void EnableValidationLayers();
        //
        std::vector<VkImageView> CreateImageViews(VkFormat format, const std::vector<VkImage> &images);
        void DestroyImageViews(const std::vector<VkImageView> &views);
        void DestroyFramebuffers(const std::vector<VkFramebuffer> &framebuffers);
    };
};