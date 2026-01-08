#include "instance.hpp"

namespace gl
{
    LogLevelType LogLevel = LogLevelType::Debug;

    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData)
    {
        if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            fmtx::Error(pCallbackData->pMessage);
        }
        else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            if (LogLevelType::Warning >= LogLevel)
                fmtx::Warn(pCallbackData->pMessage);
        }
        else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        {
            if (LogLevelType::Info >= LogLevel)
                fmtx::Info(pCallbackData->pMessage);
        }
        else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        {
            if (LogLevelType::Debug >= LogLevel)
                fmtx::Debug(pCallbackData->pMessage);
        }

        return VK_FALSE;
    }

    std::vector<VkExtensionProperties> GetSupportedInstanceExtensions()
    {
        uint32_t count = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
        std::vector<VkExtensionProperties> extensions(count);
        vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());
        return extensions;
    }

    std::vector<VkLayerProperties> GetAvailableValidationLayers()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        return std::move(availableLayers);
    }

    bool CheckValidationLayersSupport(const std::vector<VkLayerProperties> &availableLayers, const CStrings &requestedLayers)
    {
        for (const char *layerName : requestedLayers)
        {
            bool layerFound = false;

            for (const auto &layerProperties : availableLayers)
            {
                if (strcmp(layerName, layerProperties.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound)
            {
                return false;
            }
        }

        return true;
    }

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            func(instance, debugMessenger, pAllocator);
        }
    }
}

gl::Instance::Instance() : debug(false),
                           handle(VK_NULL_HANDLE),
                           createInfo({}),
                           appInfo({}),
                           debugMessenger(VK_NULL_HANDLE),
                           debugMessengerCreateInfo({})
{
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "DIYApp";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "DIYE";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;
    appInfo.pNext = nullptr;

    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = 0;
    createInfo.ppEnabledExtensionNames = nullptr;
#ifdef __APPLE__
    createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;
}

bool gl::Instance::Create()
{
    VkResult result = vkCreateInstance(&createInfo, nullptr, &handle);
    if (result != VK_SUCCESS)
    {
        fmtx::Error("Failed to create Vulkan instance");
        return false;
    }
    fmtx::Info("Vulkan instance created");

    if (debug)
    {
        if (CreateDebugUtilsMessengerEXT(handle, &debugMessengerCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS)
        {
            fmtx::Warn("Failed to set up debug messenger");
        }
    }

    return true;
}

void gl::Instance::Destroy()
{
    if (debugMessenger != VK_NULL_HANDLE)
        DestroyDebugUtilsMessengerEXT(handle, debugMessenger, nullptr);

    if (handle != VK_NULL_HANDLE)
        vkDestroyInstance(handle, nullptr);
}

void gl::Instance::SetExtensions(const CStrings &extensions)
{
    this->extensions = extensions;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(this->extensions.size());
    createInfo.ppEnabledExtensionNames = this->extensions.data();
}

void gl::Instance::EnableValidationLayers()
{
    validationLayers = CStrings({"VK_LAYER_KHRONOS_validation"});
    if (validationLayers.size() > 0)
    {
        if (!CheckValidationLayersSupport(GetAvailableValidationLayers(), validationLayers))
        {
            fmtx::Warn("Validation layers not supported");
            return;
        }

        fmtx::Info("Validation layers supported");

        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }

    debug = true;
    debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugMessengerCreateInfo.pfnUserCallback = debugCallback;
    debugMessengerCreateInfo.pUserData = nullptr; // Optional
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugMessengerCreateInfo;
}
