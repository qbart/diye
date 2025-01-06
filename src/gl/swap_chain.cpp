#include "swap_chain.hpp"

#include "image.hpp"
#include <algorithm>

namespace gl
{

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

    SwapChain::SwapChain()
    {
    }

    bool SwapChain::Create(const Device &device, const Surface &surface, const PhysicalDevice &physicalDevice)
    {
        images.clear();

        gl::SwapChainSupportDetails swapChainSupport = physicalDevice.swapChainSupport;
        // at this point we know that this format is available
        VkSurfaceFormatKHR surfaceFormat = {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
        // guaranteed to be available
        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
        extent = SelectSwapExtent(swapChainSupport.capabilities, surface.window);
        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
            imageCount = swapChainSupport.capabilities.maxImageCount;

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface.handle;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // VK_IMAGE_USAGE_TRANSFER_DST_BIT
        createInfo.pNext = nullptr;

        uint32_t queueFamilyIndices[] = {
            physicalDevice.queueFamilyIndices.graphicsFamily.value(),
            physicalDevice.queueFamilyIndices.presentFamily.value()};

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

        if (vkCreateSwapchainKHR(device.handle, &createInfo, nullptr, &handle) != VK_SUCCESS)
        {
            fmtx::Error("Failed to create swap chain");
            return false;
        }
        imageFormat = surfaceFormat.format;

        fmtx::Info("Swap chain created");

        std::vector<VkImage> imageHandles;

        vkGetSwapchainImagesKHR(device.handle, handle, &imageCount, nullptr);
        imageHandles.resize(imageCount);
        vkGetSwapchainImagesKHR(device.handle, handle, &imageCount, imageHandles.data());
        images.reserve(imageCount);

        for (auto &imageHandle : imageHandles)
        {
            Image image;
            image.handle = imageHandle;
            images.emplace_back(image);
        }

        return true;
    }

    void SwapChain::Destroy(const Device &device)
    {
        if (handle != VK_NULL_HANDLE)
            vkDestroySwapchainKHR(device.handle, handle, nullptr);
    }

    VkResult SwapChain::AcquireNextImage(const Device &device, uint32_t *imageIndex, VkSemaphore semaphore, VkFence fence) const
    {
        return vkAcquireNextImageKHR(device.handle, handle, UINT64_MAX, semaphore, fence, imageIndex);
    }
}