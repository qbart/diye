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

    void Buffer::Usage(VkBufferUsageFlags usage)
    {
        createInfo.usage = usage;
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

    void CopyBuffer(const Device &device, VkCommandPool commandPool, VkQueue queue, const Buffer &srcBuffer, Buffer &dstBuffer, VkDeviceSize size)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device.handle, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer.handle, dstBuffer.handle, 1, &copyRegion);

        vkEndCommandBuffer(commandBuffer);
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue);
        vkFreeCommandBuffers(device.handle, commandPool, 1, &commandBuffer);
    }
}