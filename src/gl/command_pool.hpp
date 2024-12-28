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
    };
}