#pragma once

#include "core.hpp"
#include "device.hpp"
#include "shader_modules.hpp"

namespace gl
{
    class RenderPass
    {
    public:
        VkRenderPass handle;
        std::vector<VkAttachmentDescription> colorAttachments;
        std::vector<VkAttachmentReference> colorAttachmentRefs;
        VkAttachmentDescription depthAttachment;
        VkAttachmentReference depthAttachmentRef;

        RenderPass();
        bool Create(const gl::Device &device, const ShaderModules &modules);
        void Destroy(const gl::Device &device);
        VkAttachmentDescription &AddColorAttachment(VkFormat format);
        VkAttachmentDescription &SetDepthAttachment(VkFormat format);
        bool HasDepthAttachment() const;

    private:
        uint32_t AttachmentsCount() const;
    };
}