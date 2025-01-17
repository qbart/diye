#pragma once

#include "core.hpp"
#include "physical_device.hpp"
#include "queue.hpp"

namespace gl
{
    class Device
    {
    public:
        VkDeviceCreateInfo createInfo;
        VkPhysicalDeviceFeatures deviceFeatures;
        VkDevice handle;
        Queue graphicsQueue;
        Queue presentQueue;
        std::vector<const char *> requiredExtensions;
        std::vector<const char *> validationLayers;

        Device();
        bool Create(const PhysicalDevice &physicalDevice);
        void Destroy();
        VkResult WaitIdle() const;
        void RequireSwapchainExtension();
        void SetRequiredExtensions(const CStrings &extensions);
        void EnableValidationLayers();
        void UpdateDescriptorSets(const std::vector<VkWriteDescriptorSet> &descriptorWrites);
        void EnableSampleRateShading();
    };
};