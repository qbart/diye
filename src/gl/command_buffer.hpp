#pragma once

#include "core.hpp"
#include "device.hpp"
#include "command_pool.hpp"
#include "render_pass.hpp"
#include "framebuffer.hpp"

namespace gl
{
    class CommandBuffer
    {
    public:
        std::vector<VkCommandBuffer> handles;
        VkCommandBufferAllocateInfo allocInfo;

        CommandBuffer();
        bool Allocate(const Device &device, const CommandPool &commandPool, uint32_t count);
        void Free(const Device &device, const CommandPool &commandPool);

        VkResult Begin(uint32_t cmdBufferIndex, VkCommandBufferUsageFlags flags = 0);
        VkResult End(uint32_t cmdBufferIndex);
        void Reset(uint32_t cmdBufferIndex, VkCommandBufferResetFlags flags = 0);
        void ClearColor(VkClearColorValue color);
        void ClearDepthStencil();
        void CmdBeginRenderPass(uint32_t cmdBufferIndex, const RenderPass &renderPass, const Framebuffer &framebuffer, VkExtent2D renderAreaExtent);
        void CmdEndRenderPass(uint32_t cmdBufferIndex);
        void CmdViewport(uint32_t cmdBufferIndex, VkOffset2D offset, VkExtent2D size, float minDepth = 0, float maxDepth = 1);
        void CmdScissor(uint32_t cmdBufferIndex, VkOffset2D offset, VkExtent2D size);

    private:
        std::vector<VkClearValue> clearValues;
    };
}
