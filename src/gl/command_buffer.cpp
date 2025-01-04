#include "command_buffer.hpp"

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
}
