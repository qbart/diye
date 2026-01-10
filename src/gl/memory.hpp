#pragma once

#include "../core/types.hpp"
#include "core.hpp"
#include "device.hpp"
#include "physical_device.hpp"

namespace gl
{
class Memory
{
public:
    VkDeviceMemory handle;
    VkMemoryAllocateInfo allocateInfo;

    Memory();

    bool Allocate(
        const PhysicalDevice &physicalDevice,
        const Device &device,
        VkMemoryRequirements requirements,
        VkMemoryPropertyFlags properties
    );
    void Free(const Device &device);

    void Map(const Device &device, VkDeviceSize offset, VkDeviceSize size);
    void Unmap(const Device &device);
    void CopyRaw(const Device &device, const void *src, VkDeviceSize size);

private:
    bool mapped;
    void *mappedData;
};
} // namespace gl