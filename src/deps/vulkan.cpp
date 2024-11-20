#include "vulkan.hpp"
#include <map>

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

    std::vector<VkExtensionProperties> GetSupportedInstanceExtensions()
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
        appInfo.apiVersion = VK_API_VERSION_1_1;
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

    void DestroyInstance(const Instance &instance)
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

            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(devices[i].device, &queueFamilyCount, nullptr);
            devices[i].queueFamilies.resize(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(devices[i].device, &queueFamilyCount, devices[i].queueFamilies.data());

            int familyIndex = 0;
            for (const auto &queueFamily : devices[i].queueFamilies)
            {
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    devices[i].queueFamilyIndices.graphicsFamily = familyIndex;

                ++familyIndex;
            }
        }

        return devices;
    }

    PhysicalDevice SelectBestPhysicalDevice(const std::vector<PhysicalDevice> &devices)
    {
        std::multimap<int, PhysicalDevice> candidates;

        for (const auto &device : devices)
        {
            int score = device.properties.limits.maxImageDimension2D;

            if (device.IsDiscreteGPU())
                score += 1000;

            candidates.insert(std::make_pair(score, device));
        }
        PhysicalDevice best;
        int score = 0;
        for (const auto &c : candidates)
        {
            if (c.first > score)
            {
                score = c.first;
                best = c.second;
            }
        }

        return best;
    }

    Device CreateDevice(const CreateDeviceInfo &info)
    {
        Device result;
        result.handle = VK_NULL_HANDLE;

        CStrings deviceExtensions;
        deviceExtensions.emplace_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME); // apple

        VkDevice device;
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = info.physicalDevice.queueFamilyIndices.graphicsFamily.value();
        queueCreateInfo.queueCount = 1;
        float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        VkPhysicalDeviceFeatures deviceFeatures{};
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.queueCreateInfoCount = 1;
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.pNext = nullptr;
        createInfo.enabledExtensionCount = 0;
        createInfo.enabledLayerCount = 0;

        if (info.validationLayers.size() > 0)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(info.validationLayers.size());
            createInfo.ppEnabledLayerNames = info.validationLayers.data();
        }
        if (deviceExtensions.size() > 0)
        {
            createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
            createInfo.ppEnabledExtensionNames = deviceExtensions.data();
        }

        if (vkCreateDevice(info.physicalDevice.device, &createInfo, nullptr, &device) != VK_SUCCESS)
        {
            return result;
        }

        result.handle = device;
        vkGetDeviceQueue(result.handle, info.physicalDevice.queueFamilyIndices.graphicsFamily.value(), 0, &result.graphicsQueue);

        return result;
    }

    void DestroyDevice(const Device &device)
    {
        if (device.handle != VK_NULL_HANDLE)
            vkDestroyDevice(device.handle, nullptr);
    }

    bool vulkan::PhysicalDevice::IsDiscreteGPU() const
    {
        return properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    }

    bool PhysicalDevice::Valid() const
    {
        return device != VK_NULL_HANDLE && queueFamilyIndices.graphicsFamily.has_value();
    }
};