#include "image_view.hpp"
#include "vulkan.hpp"

namespace gl
{
    ImageView::ImageView() : createInfo({})
    {
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
    }

    bool ImageView::Create(const Device &device, const Image &image, VkFormat format)
    {
        createInfo.image = image.handle;
        createInfo.format = format;
        createInfo.subresourceRange.levelCount = image.createInfo.mipLevels;
        if (vkCreateImageView(device.handle, &createInfo, nullptr, &handle) == VK_SUCCESS)
        {
            if (!label.empty())
                vk::SetObjectName(device.handle, (uint64_t)handle, VK_OBJECT_TYPE_IMAGE_VIEW, label);

            return true;
        }
        else
        {
            fmtx::Error("Failed to create image view");
            return false;
        }
    }

    void ImageView::AspectMask(VkImageAspectFlags flags)
    {
        createInfo.subresourceRange.aspectMask = flags;
    }

    void ImageView::AspectMaskDepth()
    {
        AspectMask(VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    void ImageView::Destroy(const Device &device)
    {
        if (handle != VK_NULL_HANDLE)
            vkDestroyImageView(device.handle, handle, nullptr);
    }
}
