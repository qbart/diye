#include "command_pool.hpp"

namespace gl
{
CommandPool::CommandPool() : handle(VK_NULL_HANDLE), createInfo({})
{
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
}

void CommandPool::TransientOnly() { createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT; }

bool CommandPool::Create(const Device &device, uint32_t queueFamilyIndex)
{
    createInfo.queueFamilyIndex = queueFamilyIndex;
    if (vkCreateCommandPool(device.handle, &createInfo, nullptr, &handle) != VK_SUCCESS)
    {
        fmtx::Error("failed to create command pool");
        return false;
    }
    fmtx::Info("Command pool created");

    return true;
}

void CommandPool::Destroy(const Device &device) { vkDestroyCommandPool(device.handle, handle, nullptr); }

VkResult CommandPool::BeginSingleTimeCommands(const Device &device, VkCommandBuffer *commandBuffer) const
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool        = handle;
    allocInfo.commandBufferCount = 1;

    vkAllocateCommandBuffers(device.handle, &allocInfo, commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    return vkBeginCommandBuffer(*commandBuffer, &beginInfo);
}

VkResult CommandPool::EndSingleTimeCommands(const Device &device, VkQueue queue, VkCommandBuffer commandBuffer) const
{
    auto result = vkEndCommandBuffer(commandBuffer);
    VkSubmitInfo submitInfo{};
    submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &commandBuffer;

    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);
    vkFreeCommandBuffers(device.handle, handle, 1, &commandBuffer);

    return result;
}
} // namespace gl