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
    std::string label;

    ImageView();
    bool Create(const Device &device, const Image &image, VkFormat format);
    void AspectMask(VkImageAspectFlags flags);
    void AspectMaskDepth();
    void Destroy(const Device &device);
};
} // namespace gl
