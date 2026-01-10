#include "framebuffer.hpp"

namespace gl
{
Framebuffer::Framebuffer() : createInfo({}), handle(VK_NULL_HANDLE) {}

bool Framebuffer::Create(const Device &device, const RenderPass &renderPass, const VkExtent2D &extent)
{
    createInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.renderPass      = renderPass.handle;
    createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    createInfo.pAttachments    = attachments.data();
    createInfo.width           = extent.width;
    createInfo.height          = extent.height;
    createInfo.layers          = 1;

    if (vkCreateFramebuffer(device.handle, &createInfo, nullptr, &handle) == VK_SUCCESS) return true;

    fmtx::Error("Failed to create framebuffer");

    return false;
}

void Framebuffer::ClearAttachments() { attachments.clear(); }

void Framebuffer::Destroy(const Device &device)
{
    if (handle != VK_NULL_HANDLE) vkDestroyFramebuffer(device.handle, handle, nullptr);
}

void Framebuffer::AddAttachment(const ImageView &imageView) { attachments.push_back(imageView.handle); }
} // namespace gl