#include "command_pool.hpp"

namespace gl
{
    CommandPool::CommandPool() : handle(VK_NULL_HANDLE),
                                 createInfo({})
    {
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    }

    void CommandPool::TransientOnly()
    {
        createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    }

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

    void CommandPool::Destroy(const Device &device)
    {
        vkDestroyCommandPool(device.handle, handle, nullptr);
    }
}