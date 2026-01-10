#pragma once

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#define VMA_VULKAN_VERSION 1003000
#include <vk_mem_alloc.h>

namespace gl
{

class Allocator
{
public:
    VmaAllocator handle;

public:
    Allocator() {};

    bool Create(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);
    void Destroy();
};

} // namespace gl
