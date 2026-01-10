#include "vma.hpp"
#include "../deps/fmt.hpp"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

namespace gl
{

bool Allocator::Create(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device)
{
    VmaVulkanFunctions vulkanFunctions    = {};
    vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr   = &vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.flags                  = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
    allocatorCreateInfo.vulkanApiVersion       = VK_API_VERSION_1_3;
    allocatorCreateInfo.physicalDevice         = physicalDevice;
    allocatorCreateInfo.device                 = device;
    allocatorCreateInfo.instance               = instance;
    allocatorCreateInfo.pVulkanFunctions       = &vulkanFunctions;

    VkResult result = vmaCreateAllocator(&allocatorCreateInfo, &handle);
    if (result != VK_SUCCESS)
    {
        fmtx::Error("Fail to create Allocator");
        return false;
    }
    fmtx::Info("Vulkan allocator created");

    return true;
}

void Allocator::Destroy()
{
    if (handle != VK_NULL_HANDLE) vmaDestroyAllocator(handle);
}

} // namespace gl
