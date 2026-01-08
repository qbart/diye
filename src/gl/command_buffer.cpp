#include "command_buffer.hpp"
#include "vulkan.hpp"

namespace gl
{
    CommandBuffer::CommandBuffer() : allocInfo({}),
                                     clearValues(0)
    {
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    }

    bool CommandBuffer::Allocate(const Device &device, const CommandPool &commandPool, uint32_t count)
    {
        handles.resize(count);
        allocInfo.commandPool = commandPool.handle;
        allocInfo.commandBufferCount = static_cast<uint32_t>(handles.size());
        if (vkAllocateCommandBuffers(device.handle, &allocInfo, handles.data()) == VK_SUCCESS)
        {
            fmtx::Info("Allocated command buffers");
            return true;
        }

        fmtx::Error("Failed to allocate command buffers");
        return false;
    }

    void CommandBuffer::Free(const Device &device, const CommandPool &commandPool)
    {
        vkFreeCommandBuffers(device.handle, commandPool.handle, 1, handles.data());
    }

    VkResult CommandBuffer::Begin(uint32_t cmdBufferIndex, VkCommandBufferUsageFlags flags)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = flags;
        beginInfo.pInheritanceInfo = nullptr;

        return vkBeginCommandBuffer(handles[cmdBufferIndex], &beginInfo);
    }

    VkResult CommandBuffer::End(uint32_t cmdBufferIndex)
    {
        return vkEndCommandBuffer(handles[cmdBufferIndex]);
    }

    void CommandBuffer::Reset(uint32_t cmdBufferIndex, VkCommandBufferResetFlags flags)
    {
        vkResetCommandBuffer(handles[cmdBufferIndex], flags);
    }

    void CommandBuffer::ClearColor(VkClearColorValue color)
    {
        VkClearValue clearValue{};
        clearValue.color = color;
        clearValues.push_back(clearValue);
    }

    void CommandBuffer::ClearDepthStencil()
    {
        VkClearValue clearValue{};
        clearValue.depthStencil = {1.0f, 0};
        clearValues.push_back(clearValue);
    }

    void CommandBuffer::CmdBeginRenderPass(uint32_t cmdBufferIndex, const RenderPass &renderPass, const Framebuffer &framebuffer, VkExtent2D renderAreaExtent)
    {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass.handle;
        renderPassInfo.framebuffer = framebuffer.handle;
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = renderAreaExtent;
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(handles[cmdBufferIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        clearValues.clear();
    }

    void CommandBuffer::CmdBeginRenderingKHR(uint32_t cmdBufferIndex, VkExtent2D renderAreaExtent, VkImageView imageView)
    {
        const VkRenderingAttachmentInfoKHR attachmentInfo {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
            .imageView = imageView,
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        };

        const VkRenderingInfoKHR renderInfo {
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
            .renderArea = VkRect2D{
                .offset = {0,0},
                .extent = renderAreaExtent,
            },
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &attachmentInfo,
        };

        vk::CmdBeginRenderingKHR(handles[cmdBufferIndex], &renderInfo);
    }

    void CommandBuffer::CmdEndRenderingKHR(uint32_t cmdBufferIndex)
    {
        vk::CmdEndRenderingKHR(handles[cmdBufferIndex]);
    }

    void CommandBuffer::CmdEndRenderPass(uint32_t cmdBufferIndex)
    {
        vkCmdEndRenderPass(handles[cmdBufferIndex]);
    }

    void CommandBuffer::CmdViewport(uint32_t cmdBufferIndex, VkOffset2D offset, VkExtent2D size, float minDepth, float maxDepth)
    {
        VkViewport viewport{};
        viewport.x = static_cast<float>(offset.x);
        viewport.y = static_cast<float>(offset.y);
        viewport.width = static_cast<float>(size.width);
        viewport.height = static_cast<float>(size.height);
        viewport.minDepth = minDepth;
        viewport.maxDepth = maxDepth;
        vkCmdSetViewport(handles[cmdBufferIndex], 0, 1, &viewport);
    }

    void CommandBuffer::CmdScissor(uint32_t cmdBufferIndex, VkOffset2D offset, VkExtent2D size)
    {
        VkRect2D scissor{};
        scissor.offset = offset;
        scissor.extent = size;
        vkCmdSetScissor(handles[cmdBufferIndex], 0, 1, &scissor);
    }

    void CommandBuffer::CmdBindGraphicsPipeline(uint32_t cmdBufferIndex, const Pipeline &pipeline)
    {
        vkCmdBindPipeline(handles[cmdBufferIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle);
    }

    void CommandBuffer::CmdBindDescriptorSet(uint32_t cmdBufferIndex, const Pipeline &pipeline, VkDescriptorSet descriptorSet)
    {
        vkCmdBindDescriptorSets(handles[cmdBufferIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, &descriptorSet, 0, nullptr);
    }

    void CommandBuffer::CmdBindVertexBuffer(uint32_t cmdBufferIndex, const Buffer &buffer, VkDeviceSize offset)
    {
        VkDeviceSize offsets[] = {offset};
        vkCmdBindVertexBuffers(handles[cmdBufferIndex], 0, 1, &buffer.handle, offsets);
    }

    void CommandBuffer::CmdBindIndexBuffer(uint32_t cmdBufferIndex, const Buffer &buffer, VkDeviceSize offset)
    {
        vkCmdBindIndexBuffer(handles[cmdBufferIndex], buffer.handle, offset, VK_INDEX_TYPE_UINT32);
    }

    void CommandBuffer::CmdDrawIndexed(uint32_t cmdBufferIndex, uint32_t indexCount)
    {
        vkCmdDrawIndexed(handles[cmdBufferIndex], indexCount, 1, 0, 0, 0);
    }

    void CommandBuffer::CmdBeginDebugLabel(uint32_t cmdBufferIndex, const char *label, const float *color)
    {
        if (vk::CmdBeginDebugUtilsLabelEXT == nullptr || label == nullptr)
            return;

        VkDebugUtilsLabelEXT labelInfo{};
        labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        labelInfo.pLabelName = label;
        const float defaultColor[4] = {0.45f, 0.8f, 0.35f, 1.0f};
        const float *inputColor = color != nullptr ? color : defaultColor;
        for (int i = 0; i < 4; ++i)
            labelInfo.color[i] = inputColor[i];

        vk::CmdBeginDebugUtilsLabelEXT(handles[cmdBufferIndex], &labelInfo);
    }

    void CommandBuffer::CmdEndDebugLabel(uint32_t cmdBufferIndex)
    {
        if (vk::CmdEndDebugUtilsLabelEXT == nullptr)
            return;

        vk::CmdEndDebugUtilsLabelEXT(handles[cmdBufferIndex]);
    }

    void CommandBuffer::CmdInsertDebugLabel(uint32_t cmdBufferIndex, const char *label, const float *color)
    {
        if (vk::CmdInsertDebugUtilsLabelEXT == nullptr || label == nullptr)
            return;

        VkDebugUtilsLabelEXT labelInfo{};
        labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        labelInfo.pLabelName = label;
        const float defaultColor[4] = {0.3f, 0.5f, 0.9f, 1.0f};
        const float *inputColor = color != nullptr ? color : defaultColor;
        for (int i = 0; i < 4; ++i)
            labelInfo.color[i] = inputColor[i];

        vk::CmdInsertDebugUtilsLabelEXT(handles[cmdBufferIndex], &labelInfo);
    }
}
