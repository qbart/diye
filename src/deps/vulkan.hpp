#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_beta.h>
#include <string>
#include <vector>
#include <optional>
#include "fmt.hpp"
#include "sdl.hpp"

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

    struct Surface
    {
        VkSurfaceKHR handle;
        SDL_Window *window;

        inline bool IsValid() const { return handle != VK_NULL_HANDLE; }
    };

    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
    };

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct PhysicalDevice
    {
        VkPhysicalDevice device;
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceFeatures features;
        std::vector<VkExtensionProperties> extensions;
        std::vector<VkQueueFamilyProperties> queueFamilies;
        QueueFamilyIndices queueFamilyIndices;
        SwapChainSupportDetails swapChainSupport;

        bool IsDiscreteGPU() const;
        bool IsValid() const;
        bool IsExtensionSupported(const CStrings &extensions) const;
        void QuerySwapChainSupport(const Surface &surface);
        void QueryQueueFamilies(const Surface &surface);
    };

    struct CreateDeviceInfo
    {
        PhysicalDevice physicalDevice;
        CStrings validationLayers;
        CStrings requiredExtensions;
    };

    struct Device
    {
        VkDevice handle;
        VkQueue graphicsQueue;
        VkQueue presentQueue;

        inline bool IsValid() const { return handle != VK_NULL_HANDLE; }
        VkResult WaitIdle() const;
    };

    struct CreateSwapChainInfo
    {
        Surface surface;
        PhysicalDevice physicalDevice;
        Device device;
        CStrings validationLayers;
    };

    struct SwapChain
    {
        VkSwapchainKHR handle;
        std::vector<VkImage> images;
        VkFormat imageFormat;
        VkExtent2D extent;

        bool IsValid() const;
    };

    struct ShaderModule
    {
        VkShaderModule handle;

        inline bool IsValid() const { return handle != VK_NULL_HANDLE; }
    };

    struct ShaderModules
    {
        ShaderModule vert;
        ShaderModule frag;
    };

    struct ShaderStage
    {
        VkShaderModule module;
        VkShaderStageFlagBits stage;
    };

    struct RenderPass
    {
        VkRenderPass handle;

        inline bool IsValid() const { return handle != VK_NULL_HANDLE; }
    };

    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData);

    std::vector<VkExtensionProperties> GetSupportedInstanceExtensions();
    std::vector<VkLayerProperties> GetAvailableValidationLayers();
    bool CheckValidationLayersSupport(const std::vector<VkLayerProperties> &availableLayers, const CStrings &requestedLayers);
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger);
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator);
    Instance CreateInstance(const CreateInstanceInfo &info, bool debug = false);
    void DestroyInstance(const Instance &instance);
    Surface CreateSurface(const Instance &instance, SDL_Window *window);
    void DestroySurface(const Instance &instance, const Surface &surface);
    std::vector<VkExtensionProperties> GetSupportedPhysicalDeviceExtensions(const VkPhysicalDevice &device);
    std::vector<PhysicalDevice> GetPhysicalDevices(const Instance &instance, const Surface &surface);
    PhysicalDevice SelectBestPhysicalDevice(const std::vector<PhysicalDevice> &devices);
    Device CreateDevice(const CreateDeviceInfo &info);
    void DestroyDevice(const Device &device);
    VkExtent2D SelectSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, SDL_Window *window);
    SwapChain CreateSwapChain(const CreateSwapChainInfo &info);
    void DestroySwapChain(const Device &device, const SwapChain &swapChain);
    std::vector<VkImageView> CreateImageViews(const Device &device, const SwapChain &swapChain);
    void DestroyImageViews(const Device &device, const std::vector<VkImageView> &views);
    void DestroyFramebuffers(const Device &device, const std::vector<VkFramebuffer> &framebuffers);
    ShaderModule CreateShaderModule(const Device &device, const std::vector<char> &code);
    void DestroyShaderModule(const Device &device, const ShaderModule &module);
    RenderPass CreateRenderPass(const Device &device, const SwapChain &swapChain, const ShaderModules &modules);
    void DestroyRenderPass(const Device &device, const RenderPass &renderPass);
};