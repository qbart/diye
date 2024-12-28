#pragma once

#include "core.hpp"
#include "physical_device.hpp"

namespace gl
{
    class RenderPass;
    class Image;
    class ImageView;

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
        std::vector<ImageView> CreateImageViews(VkFormat format, const std::vector<Image> &images);
        void DestroyImageViews(std::vector<ImageView> &views);
        std::vector<VkFramebuffer> CreateFramebuffers(const RenderPass &renderPass, const std::vector<ImageView> &views, const VkExtent2D &extent);
        void DestroyFramebuffers(const std::vector<VkFramebuffer> &framebuffers);
    };
};