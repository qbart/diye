#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include "fmt.hpp"

namespace vulkan
{
    using CStrings = std::vector<const char *>;
    using Strings = std::vector<std::string>;

    enum LogLevelType
    {
        Error = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        Warning = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
        Info = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
        Debug = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
    };

    extern LogLevelType LogLevel;

    struct CreateInstanceInfo
    {
        std::string title;
        CStrings extensions;
        CStrings validationLayers;
    };

    struct Instance
    {
        VkInstance handle;
        VkDebugUtilsMessengerEXT debugMessenger;

        inline bool IsValid() const { return handle != VK_NULL_HANDLE; }
    };

    struct PhysicalDevice
    {
        VkPhysicalDevice device;
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceFeatures features;

        bool IsDiscreteGPU() const;
    };

    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData);

    std::vector<VkExtensionProperties> GetInstanceExtensions();
    std::vector<VkLayerProperties> GetAvailableValidationLayers();
    bool CheckValidationLayersSupport(const std::vector<VkLayerProperties> &availableLayers, const CStrings &requestedLayers);
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger);
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator);
    Instance CreateInstance(const CreateInstanceInfo &info, bool debug = false);
    void DestroyInstance(Instance &instance);
    std::vector<PhysicalDevice> GetPhysicalDevices(const Instance &instance);
};