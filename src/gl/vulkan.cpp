#include "vulkan.hpp"

namespace vk
{

void InitFunctions(VkDevice device)
{
    CmdBeginRenderingKHR = reinterpret_cast<PFN_vkCmdBeginRenderingKHR>(vkGetDeviceProcAddr(device, "vkCmdBeginRenderingKHR"));
    CmdEndRenderingKHR = reinterpret_cast<PFN_vkCmdEndRenderingKHR>( vkGetDeviceProcAddr(device, "vkCmdEndRenderingKHR"));
}

void ImageTransitionLayout(VkDevice device, VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    else
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags pipelineSrcStageMask;
    VkPipelineStageFlags pipelineDstStageMask;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        pipelineSrcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        pipelineDstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        pipelineSrcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        pipelineDstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        pipelineSrcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        pipelineDstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
    {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = 0;

        pipelineSrcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        pipelineDstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }
    else
    {
        fmtx::Error("unsupported layout transition");
        return;
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        pipelineSrcStageMask,
        pipelineDstStageMask,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);
}

}

