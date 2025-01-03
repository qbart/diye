#pragma once

#include "core.hpp"
#include "device.hpp"
#include "image.hpp"

namespace gl
{
    class ImageView
    {
    public:
        VkImageView handle;
        VkImageViewCreateInfo createInfo;

        ImageView();
        bool Create(const Device &device, const Image &image, VkFormat format);
        void AspectMask(VkImageAspectFlags flags);
        void Destroy(const Device &device);
    };
}