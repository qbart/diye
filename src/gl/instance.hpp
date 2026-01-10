#pragma once

#include "core.hpp"

namespace gl
{
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData
);

enum LogLevelType
{
    Error   = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
    Warning = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
    Info    = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
    Debug   = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
};

extern LogLevelType LogLevel;

std::vector<VkExtensionProperties> GetSupportedInstanceExtensions();
std::vector<VkLayerProperties> GetAvailableValidationLayers();
bool CheckValidationLayersSupport(
    const std::vector<VkLayerProperties> &availableLayers,
    const CStrings &requestedLayers
);

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger
);
void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks *pAllocator
);

class Instance
{
public:
    VkInstanceCreateInfo createInfo;
    VkApplicationInfo appInfo;
    VkInstance handle;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo;
    CStrings validationLayers;
    CStrings extensions;

    Instance();

    bool Create();
    void Destroy();
    void SetExtensions(const CStrings &extensions);
    void EnableValidationLayers();

private:
    bool debug;
};
}; // namespace gl
