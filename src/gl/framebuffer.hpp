#pragma once

#include "core.hpp"
#include "device.hpp"
#include "image_view.hpp"
#include "render_pass.hpp"

namespace gl
{
class Framebuffer
{
public:
    VkFramebufferCreateInfo createInfo;
    VkFramebuffer handle;
    std::vector<VkImageView> attachments;

    Framebuffer();
    bool Create(const Device &device, const RenderPass &renderPass, const VkExtent2D &extent);
    void ClearAttachments();
    void Destroy(const Device &device);
    void AddAttachment(const ImageView &imageView);
};
}; // namespace gl