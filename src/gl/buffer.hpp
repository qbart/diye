#pragma once

#include "core.hpp"
#include "device.hpp"

namespace gl
{
    class Memory;

    class Buffer
    {
    public:
        VkBuffer handle;
        VkBufferCreateInfo createInfo;

        Buffer();

        bool Create(const Device &device, VkDeviceSize size);
        void Destroy(const Device &device);
        VkMemoryRequirements MemoryRequirements(const Device &device) const;
        VkDeviceSize Size() const;
        void BindMemory(const Device &device, const Memory &memory, VkDeviceSize offset);
    };
}