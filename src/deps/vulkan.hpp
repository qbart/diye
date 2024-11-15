#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace vulkan
{
    std::vector<VkExtensionProperties> GetInstanceExtensions()
    {
        uint32_t count = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
        std::vector<VkExtensionProperties> extensions(count);
        vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());
        return extensions;
    }
};