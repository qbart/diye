#include "buffer.hpp"
#include "memory.hpp"

namespace gl
{
    Buffer::Buffer() : handle(VK_NULL_HANDLE),
                       createInfo({})
    {
        createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    bool Buffer::Create(const Device &device, VkDeviceSize size)
    {
        createInfo.size = size;
        if (vkCreateBuffer(device.handle, &createInfo, nullptr, &handle) != VK_SUCCESS)
        {
            fmtx::Error("failed to create buffer");
            return false;
        }
        return true;
    }

    void Buffer::Destroy(const Device &device)
    {
        if (handle != VK_NULL_HANDLE)
            vkDestroyBuffer(device.handle, handle, nullptr);
    }

    VkMemoryRequirements Buffer::MemoryRequirements(const Device &device) const
    {
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device.handle, handle, &memRequirements);
        return memRequirements;
    }

    VkDeviceSize Buffer::Size() const
    {
        return createInfo.size;
    }

    void Buffer::BindMemory(const Device &device, const Memory &memory, VkDeviceSize offset)
    {
        vkBindBufferMemory(device.handle, handle, memory.handle, offset);
    }
}