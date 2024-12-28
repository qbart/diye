#pragma once

#include "core.hpp"
#include "device.hpp"

namespace gl
{
    class ImageView
    {
    public:
        VkImageView handle;
        VkImageViewCreateInfo createInfo;

        ImageView();
    };
}