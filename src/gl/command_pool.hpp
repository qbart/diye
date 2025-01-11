#pragma once

#include "core.hpp"
#include "device.hpp"

namespace gl
{
    class CommandPool
    {
    public:
        VkCommandPool handle;
        VkCommandPoolCreateInfo createInfo;

        CommandPool();
        void TransientOnly();
        bool Create(const Device &device, uint32_t queueFamilyIndex);
        void Destroy(const Device &device);
        VkResult BeginSingleTimeCommands(const Device &device, VkCommandBuffer *commandBuffer) const;
        VkResult EndSingleTimeCommands(const Device &device, VkQueue queue, VkCommandBuffer commandBuffer) const;
    };
}