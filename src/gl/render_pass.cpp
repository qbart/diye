#include "render_pass.hpp"

#include "device.hpp"
#include "vulkan.hpp"
#include "../deps/fmt.hpp"

gl::RenderPass::RenderPass() : handle(VK_NULL_HANDLE),
                               depthAttachment{},
                               depthAttachmentRef{}
{
    depthAttachment.format = VK_FORMAT_UNDEFINED;
}

bool gl::RenderPass::Create(const gl::Device &device, const ShaderModules &modules)
{
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
    subpass.pColorAttachments = colorAttachmentRefs.data();
    if (resolveAttachmentRefs.size() > 0)
        subpass.pResolveAttachments = resolveAttachmentRefs.data();

    if (HasDepthAttachment())
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;

    // for depth only this needs to be adjusted
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    if (HasDepthAttachment())
        dependency.srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

    dependency.srcAccessMask = 0;

    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    if (HasDepthAttachment())
        dependency.dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    if (HasDepthAttachment())
        dependency.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::vector<VkAttachmentDescription> attachments;
    attachments.reserve(AttachmentsCount());
    for (size_t i = 0; i < colorAttachments.size(); i++)
        attachments.push_back(colorAttachments[i]);
    if (HasDepthAttachment())
        attachments.push_back(depthAttachment);
    for (size_t i = 0; i < resolveAttachments.size(); i++)
        attachments.push_back(resolveAttachments[i]);

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = AttachmentsCount();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device.handle, &renderPassInfo, nullptr, &handle) != VK_SUCCESS)
    {
        fmtx::Error("Failed to create render pass");
        return false;
    }
    fmtx::Info("Render pass created");
    if (!label.empty())
        vk::SetObjectName(device.handle, (uint64_t)handle, VK_OBJECT_TYPE_RENDER_PASS, label);

    return true;
}

void gl::RenderPass::Destroy(const gl::Device &device)
{
    if (handle != VK_NULL_HANDLE)
        vkDestroyRenderPass(device.handle, handle, nullptr);
}

VkAttachmentDescription &gl::RenderPass::AddColorAttachment(VkFormat format)
{
    VkAttachmentDescription attachment{};
    attachment.format = format;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachments.push_back(attachment);

    VkAttachmentReference attachmentRef{};
    attachmentRef.attachment = AttachmentsCount() - 1;
    attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachmentRefs.push_back(attachmentRef);

    return colorAttachments.back();
}

VkAttachmentDescription &gl::RenderPass::SetDepthAttachment(VkFormat format)
{
    depthAttachment.format = format;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    depthAttachmentRef.attachment = AttachmentsCount() - 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    return depthAttachment;
}

VkAttachmentDescription &gl::RenderPass::AddResolveAttachment(VkFormat format)
{
    VkAttachmentDescription colorAttachmentResolve{};
    colorAttachmentResolve.format = format;
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    resolveAttachments.push_back(colorAttachmentResolve);

    VkAttachmentReference attachmentRef{};
    attachmentRef.attachment = AttachmentsCount() - 1;
    attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    resolveAttachmentRefs.push_back(attachmentRef);

    return resolveAttachments.back();
}

bool gl::RenderPass::HasDepthAttachment() const
{
    return depthAttachment.format != VK_FORMAT_UNDEFINED;
}

uint32_t gl::RenderPass::AttachmentsCount() const
{
    uint32_t dephtAttachment = HasDepthAttachment() ? 1 : 0;
    return static_cast<uint32_t>(colorAttachments.size() + dephtAttachment) + static_cast<uint32_t>(resolveAttachments.size());
}
