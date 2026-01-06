#include "image.hpp"

namespace gl
{
    Image::Image() : createInfo({})
    {
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        createInfo.imageType = VK_IMAGE_TYPE_2D;
        createInfo.extent.depth = 1;
        createInfo.mipLevels = 1;
        createInfo.arrayLayers = 1;
        createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        createInfo.flags = 0;
    }

    void Image::Usage(VkImageUsageFlags usage)
    {
        createInfo.usage = usage;
    }

    void Image::MipLevels(uint32_t mipLevels)
    {
        createInfo.mipLevels = mipLevels;
    }

    void Image::Samples(VkSampleCountFlagBits samples)
    {
        createInfo.samples = samples;
    }

    void Image::UsageDepthOnly()
    {
        Usage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

    bool Image::Create(const Device &device, VkExtent2D extent, VkFormat format)
    {
        createInfo.extent.width = extent.width;
        createInfo.extent.height = extent.height;
        createInfo.format = format;
        if (vkCreateImage(device.handle, &createInfo, nullptr, &handle) == VK_SUCCESS)
        {
            return true;
        }
        fmtx::Error("Failed to create image");

        return false;
    }

    void Image::Destroy(const Device &device)
    {
        vkDestroyImage(device.handle, handle, nullptr);
    }

    bool Image::BindMemory(const Device &device, const Memory &memory, VkDeviceSize offset)
    {
       return vkBindImageMemory(device.handle, handle, memory.handle, offset) == VK_SUCCESS;
    }

    VkMemoryRequirements Image::MemoryRequirements(const Device &device) const
    {
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device.handle, handle, &memRequirements);
        return memRequirements;
    }

    bool Image::CopyFromBuffer(const Device &device, const CommandPool &commandPool, VkQueue queue, const Buffer &buffer, VkExtent2D imageSize)
    {
        VkCommandBuffer commandBuffer;
        auto result = commandPool.BeginSingleTimeCommands(device, &commandBuffer);
        if (result != VK_SUCCESS)
        {
            fmtx::Error("Failed to begin single time commands");
            return false;
        }

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = {0, 0, 0};
        region.imageExtent = {imageSize.width, imageSize.height, 1};

        vkCmdCopyBufferToImage(
            commandBuffer,
            buffer.handle,
            handle,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region);

        result = commandPool.EndSingleTimeCommands(device, queue, commandBuffer);
        if (result != VK_SUCCESS)
        {
            fmtx::Error("Failed to end single time commands");
            return false;
        }
        return true;
    }

    bool Image::TransitionLayout(const Device &device, const CommandPool &commandPool, VkQueue queue, VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkCommandBuffer commandBuffer;
        auto result = commandPool.BeginSingleTimeCommands(device, &commandBuffer);
        if (result != VK_SUCCESS)
        {
            fmtx::Error("Failed to begin single time commands");
            return false;
        }

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = handle;
        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        else
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = createInfo.mipLevels;
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
            return false;
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            pipelineSrcStageMask,
            pipelineDstStageMask,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        result = commandPool.EndSingleTimeCommands(device, queue, commandBuffer);
        if (result != VK_SUCCESS)
        {
            fmtx::Error("Failed to end single time commands");
            return false;
        }
        return true;
    }

    bool Image::GenerateMipmaps(const Device &device, const CommandPool &commandPool, VkQueue queue, VkFilter filter)
    {
        VkCommandBuffer commandBuffer;
        auto result = commandPool.BeginSingleTimeCommands(device, &commandBuffer);
        if (result != VK_SUCCESS)
        {
            fmtx::Error("Failed to begin single time commands");
            return false;
        }

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = handle;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;
        int32_t mipWidth = createInfo.extent.width;
        int32_t mipHeight = createInfo.extent.height;

        for (uint32_t i = 1; i < createInfo.mipLevels; i++)
        {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 0,
                                 0, nullptr,
                                 0, nullptr,
                                 1, &barrier);

            VkImageBlit blit{};
            blit.srcOffsets[0] = {0, 0, 0};
            blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.dstOffsets[0] = {0, 0, 0};
            blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;

            vkCmdBlitImage(commandBuffer,
                           handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1, &blit,
                           filter);

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                 0,
                                 0, nullptr,
                                 0, nullptr,
                                 1, &barrier);
            if (mipWidth > 1)
                mipWidth /= 2;
            if (mipHeight > 1)
                mipHeight /= 2;
        }

        barrier.subresourceRange.baseMipLevel = createInfo.mipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);

        result = commandPool.EndSingleTimeCommands(device, queue, commandBuffer);
        if (result != VK_SUCCESS)
        {
            fmtx::Error("Failed to end single time commands");
            return false;
        }
        return true;
    }
}
