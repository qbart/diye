#include "vulkan.hpp"
#include <map>
#include <set>
#include <algorithm>

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

    Surface CreateSurface(const Instance &instance, SDL_Window *window)
    {
        Surface surface;
        surface.handle = VK_NULL_HANDLE;
        surface.window = window;
        VkSurfaceKHR handle;

        if (!sdl::CreateVulkanSurface(window, instance.handle, &handle))
            return surface;

        surface.handle = handle;
        return surface;
    }

    void DestroySurface(const Instance &instance, const Surface &surface)
    {
        if (surface.handle != VK_NULL_HANDLE)
            vkDestroySurfaceKHR(instance.handle, surface.handle, nullptr);
    }

    std::vector<VkExtensionProperties> GetSupportedPhysicalDeviceExtensions(const VkPhysicalDevice &device)
    {
        uint32_t count = 0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
        std::vector<VkExtensionProperties> extensions(count);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &count, extensions.data());
        return extensions;
    }

    std::vector<PhysicalDevice> GetPhysicalDevices(const Instance &instance, const Surface &surface)
    {
        std::vector<VkPhysicalDevice> enumDevices;
        uint32_t count;
        vkEnumeratePhysicalDevices(instance.handle, &count, nullptr);

        enumDevices.resize(count);
        vkEnumeratePhysicalDevices(instance.handle, &count, enumDevices.data());

        std::vector<PhysicalDevice> devices(enumDevices.size());
        for (int i = 0; i < devices.size(); ++i)
        {
            // device
            devices[i].device = enumDevices[i];
            devices[i].extensions = GetSupportedPhysicalDeviceExtensions(devices[i].device);
            vkGetPhysicalDeviceProperties(devices[i].device, &devices[i].properties);
            vkGetPhysicalDeviceFeatures(devices[i].device, &devices[i].features);

            devices[i].QuerySwapChainSupport(surface);
            devices[i].QueryQueueFamilies(surface);
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
#ifdef __APPLE__
        deviceExtensions.emplace_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif
        for (const auto &ext : info.requiredExtensions)
            deviceExtensions.emplace_back(ext);

        VkDevice device;

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {
            info.physicalDevice.queueFamilyIndices.graphicsFamily.value(),
            info.physicalDevice.queueFamilyIndices.presentFamily.value()};
        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
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
        vkGetDeviceQueue(result.handle, info.physicalDevice.queueFamilyIndices.presentFamily.value(), 0, &result.presentQueue);

        return result;
    }

    void DestroyDevice(const Device &device)
    {
        if (device.handle != VK_NULL_HANDLE)
            vkDestroyDevice(device.handle, nullptr);
    }

    VkResult Device::WaitIdle() const
    {
        return vkDeviceWaitIdle(handle);
    }

    VkExtent2D SelectSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, SDL_Window *window)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            fmtx::Debug(fmt::format("Using fixed Extent from surface capabilities {}x{}", capabilities.currentExtent.width, capabilities.currentExtent.height));
            return capabilities.currentExtent;
        }
        else
        {
            VkExtent2D actualExtent = sdl::GetVulkanFramebufferSize(window);
            fmtx::Debug(fmt::format("Framebuffer size: {}x{}", actualExtent.width, actualExtent.height));
            fmtx::Debug(fmt::format("Min image extent: {}x{}", capabilities.minImageExtent.width, capabilities.minImageExtent.height));
            fmtx::Debug(fmt::format("Max image extent: {}x{}", capabilities.maxImageExtent.width, capabilities.maxImageExtent.height));

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    SwapChain CreateSwapChain(const CreateSwapChainInfo &info)
    {
        SwapChainSupportDetails swapChainSupport = info.physicalDevice.swapChainSupport;
        // at this point we know that this format is available
        VkSurfaceFormatKHR surfaceFormat = {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
        // guaranteed to be available
        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
        VkExtent2D extent = SelectSwapExtent(swapChainSupport.capabilities, info.surface.window);
        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
            imageCount = swapChainSupport.capabilities.maxImageCount;

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = info.surface.handle;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // VK_IMAGE_USAGE_TRANSFER_DST_BIT
        createInfo.pNext = nullptr;

        uint32_t queueFamilyIndices[] = {
            info.physicalDevice.queueFamilyIndices.graphicsFamily.value(),
            info.physicalDevice.queueFamilyIndices.presentFamily.value()};

        if (queueFamilyIndices[0] != queueFamilyIndices[1])
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }
        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        SwapChain swapChain;
        if (vkCreateSwapchainKHR(info.device.handle, &createInfo, nullptr, &swapChain.handle) != VK_SUCCESS)
        {
            swapChain.handle = VK_NULL_HANDLE;
            return swapChain;
        }
        swapChain.imageFormat = surfaceFormat.format;
        swapChain.extent = extent;

        vkGetSwapchainImagesKHR(info.device.handle, swapChain.handle, &imageCount, nullptr);
        swapChain.images.resize(imageCount);
        vkGetSwapchainImagesKHR(info.device.handle, swapChain.handle, &imageCount, swapChain.images.data());

        return swapChain;
    }

    void DestroySwapChain(const Device &device, const SwapChain &swapChain)
    {
        if (swapChain.handle != VK_NULL_HANDLE)
            vkDestroySwapchainKHR(device.handle, swapChain.handle, nullptr);
    }

    std::vector<VkImageView> CreateImageViews(const Device &device, const SwapChain &swapChain)
    {
        std::vector<VkImageView> views;
        std::vector<bool> valid;
        views.resize(swapChain.images.size());
        valid.resize(swapChain.images.size());
        bool allValid = true;

        for (size_t i = 0; i < swapChain.images.size(); i++)
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChain.images[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = swapChain.imageFormat;

            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            valid[i] = vkCreateImageView(device.handle, &createInfo, nullptr, &views[i]) == VK_SUCCESS;
            if (!valid[i])
                allValid = false;
        }

        if (!allValid)
        {
            for (size_t i = 0; i < views.size(); i++)
            {
                if (valid[i])
                    vkDestroyImageView(device.handle, views[i], nullptr);
            }
            views.clear();
        }

        return views;
    }

    void DestroyImageViews(const Device &device, const std::vector<VkImageView> &views)
    {
        for (auto imageView : views)
        {
            vkDestroyImageView(device.handle, imageView, nullptr);
        }
    }

    ShaderModule CreateShaderModule(const Device &device, const std::vector<char> &code)
    {
        ShaderModule mod;

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device.handle, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
            mod.handle = VK_NULL_HANDLE;
        else
            mod.handle = shaderModule;

        return mod;
    }

    void DestroyShaderModule(const Device &device, const ShaderModule &module)
    {
        if (module.handle != VK_NULL_HANDLE)
            vkDestroyShaderModule(device.handle, module.handle, nullptr);
    }

    RenderPass CreateRenderPass(const Device &device, const SwapChain &swapChain, const ShaderModules &modules)
    {
        RenderPass renderPass;

        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChain.imageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(device.handle, &renderPassInfo, nullptr, &renderPass.handle) != VK_SUCCESS)
        {
            renderPass.handle = VK_NULL_HANDLE;
        }

        return renderPass;
    }

    void DestroyRenderPass(const Device &device, const RenderPass &renderPass)
    {
        if (renderPass.handle != VK_NULL_HANDLE)
            vkDestroyRenderPass(device.handle, renderPass.handle, nullptr);
    }

    bool vulkan::SwapChain::IsValid() const
    {
        return handle != VK_NULL_HANDLE;
    }

    bool PhysicalDevice::IsDiscreteGPU() const
    {
        return properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    }

    bool PhysicalDevice::IsValid() const
    {
        bool valid = device != VK_NULL_HANDLE;
        bool complete = queueFamilyIndices.graphicsFamily.has_value() && queueFamilyIndices.presentFamily.has_value();
        bool deviceExtensionsSupported = IsExtensionSupported({VK_KHR_SWAPCHAIN_EXTENSION_NAME});
        bool swapChainAdequate = false;
        bool formatAvailable = false;
        if (deviceExtensionsSupported)
        {
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
            for (const auto &availableFormat : swapChainSupport.formats)
            {
                if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                {
                    formatAvailable = true;
                    break;
                }
            }
        }
        return valid && complete && deviceExtensionsSupported && swapChainAdequate && formatAvailable;
    }

    bool PhysicalDevice::IsExtensionSupported(const CStrings &checkExtensions) const
    {
        std::set<std::string> requiredExtensions(checkExtensions.begin(), checkExtensions.end());

        for (const auto &ext : extensions)
        {
            requiredExtensions.erase(ext.extensionName);
        }

        return requiredExtensions.empty();
    }

    void PhysicalDevice::QuerySwapChainSupport(const Surface &surface)
    {
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface.handle, &swapChainSupport.capabilities);
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface.handle, &formatCount, nullptr);
        if (formatCount != 0)
        {
            swapChainSupport.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface.handle, &formatCount, swapChainSupport.formats.data());
        }
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface.handle, &presentModeCount, nullptr);
        if (presentModeCount != 0)
        {
            swapChainSupport.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface.handle, &presentModeCount, swapChainSupport.presentModes.data());
        }
    }

    void PhysicalDevice::QueryQueueFamilies(const Surface &surface)
    {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        queueFamilies.resize(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int familyIndex = 0;
        for (const auto &queueFamily : queueFamilies)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                queueFamilyIndices.graphicsFamily = familyIndex;

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(
                device,
                familyIndex,
                surface.handle,
                &presentSupport);

            if (presentSupport)
                queueFamilyIndices.presentFamily = familyIndex;

            //
            ++familyIndex;
        }
    }
};