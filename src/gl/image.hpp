#pragma once

#include "core.hpp"
#include "device.hpp"
#include "buffer.hpp"
#include "memory.hpp"
#include "command_pool.hpp"

namespace gl
{
    class Image
    {
    public:
        VkImage handle;
        VkImageCreateInfo createInfo;

        Image();
        void Usage(VkImageUsageFlags usage);
        bool Create(const Device &device, VkExtent2D extent, VkFormat format);
        void Destroy(const Device &device);
        void BindMemory(const Device &device, const Memory &memory, VkDeviceSize offset);
        VkMemoryRequirements MemoryRequirements(const Device &device) const;
        void CopyFromBuffer(const Device &device, const CommandPool &commandPool, VkQueue queue, const Buffer &buffer, VkExtent2D imageSize);
        bool TransitionLayout(const Device &device, const CommandPool &commandPool, VkQueue queue, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    };
}