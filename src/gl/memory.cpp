#include "memory.hpp"

namespace gl
{
    Memory::Memory() : handle(VK_NULL_HANDLE),
                       allocateInfo({}),
                       mapped(false)
    {
        allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    }

    bool Memory::Allocate(const PhysicalDevice &physicalDevice, const Device &device, VkMemoryRequirements requirements, VkMemoryPropertyFlags properties)
    {
        allocateInfo.allocationSize = requirements.size;
        allocateInfo.memoryTypeIndex = physicalDevice.FindMemoryType(requirements.memoryTypeBits, properties);
        if (allocateInfo.memoryTypeIndex == -1)
        {
            fmtx::Error("failed to find suitable memory type");
            return false;
        }
        if (vkAllocateMemory(device.handle, &allocateInfo, nullptr, &handle) != VK_SUCCESS)
        {
            fmtx::Error("failed to allocate memory");
            return false;
        }

        return true;
    }

    void Memory::Free(const Device &device)
    {
        vkFreeMemory(device.handle, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }

    void Memory::Map(const Device &device, VkDeviceSize offset, VkDeviceSize size)
    {
        vkMapMemory(device.handle, handle, offset, size, 0, &mappedData);
        mapped = true;
    }

    void Memory::Unmap(const Device &device)
    {
        vkUnmapMemory(device.handle, handle);
        mapped = false;
    }

    void Memory::CopyRaw(const Device &device, const void *src, VkDeviceSize size)
    {
        if (!mapped)
        {
            fmtx::Error("memory not mapped");
            return;
        }
        memcpy(mappedData, src, (size_t)size);
    }
}