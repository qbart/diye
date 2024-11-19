#include "vulkan.hpp"

namespace vulkan
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

    std::vector<VkExtensionProperties> GetInstanceExtensions()
    {
        uint32_t count = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
        std::vector<VkExtensionProperties> extensions(count);
        vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());
        return std::move(extensions);
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

    Instance CreateInstance(const CreateInstanceInfo &info, bool debug)
    {
        Instance instance;
        instance.handle = VK_NULL_HANDLE;
        instance.debugMessenger = VK_NULL_HANDLE;
        VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};

        VkApplicationInfo appInfo;
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = info.title.c_str();
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;
        appInfo.pNext = nullptr;

        VkInstanceCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = info.extensions.size();
        createInfo.ppEnabledExtensionNames = info.extensions.data();
        createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;

        if (debug)
        {
            if (info.validationLayers.size() > 0)
            {
                if (!CheckValidationLayersSupport(GetAvailableValidationLayers(), info.validationLayers))
                    return instance;

                createInfo.enabledLayerCount = static_cast<uint32_t>(info.validationLayers.size());
                createInfo.ppEnabledLayerNames = info.validationLayers.data();
            }

            debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugMessengerCreateInfo.pfnUserCallback = debugCallback;
            debugMessengerCreateInfo.pUserData = nullptr; // Optional
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugMessengerCreateInfo;
        }

        VkResult result = vkCreateInstance(&createInfo, nullptr, &instance.handle);
        if (result != VK_SUCCESS)
            return instance;

        if (debug)
        {
            if (CreateDebugUtilsMessengerEXT(instance.handle, &debugMessengerCreateInfo, nullptr, &instance.debugMessenger) != VK_SUCCESS)
            {
                fmtx::Warn("Failed to set up debug messenger");
            }
        }

        return instance;
    }

    void DestroyInstance(Instance &instance)
    {
        if (instance.debugMessenger != VK_NULL_HANDLE)
            DestroyDebugUtilsMessengerEXT(instance.handle, instance.debugMessenger, nullptr);

        if (instance.handle != VK_NULL_HANDLE)
            vkDestroyInstance(instance.handle, nullptr);
    }

    std::vector<PhysicalDevice> GetPhysicalDevices(const Instance &instance)
    {
        std::vector<VkPhysicalDevice> enumDevices;
        uint32_t count;
        vkEnumeratePhysicalDevices(instance.handle, &count, nullptr);

        enumDevices.resize(count);
        vkEnumeratePhysicalDevices(instance.handle, &count, enumDevices.data());

        std::vector<PhysicalDevice> devices(enumDevices.size());
        for (int i = 0; i < devices.size(); ++i)
        {
            devices[i].device = enumDevices[i];
            vkGetPhysicalDeviceProperties(devices[i].device, &devices[i].properties);
            vkGetPhysicalDeviceFeatures(devices[i].device, &devices[i].features);
        }

        return devices;
    }

    bool vulkan::PhysicalDevice::IsDiscreteGPU() const
    {
        return properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    }
};