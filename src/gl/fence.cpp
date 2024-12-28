#include "fence.hpp"

namespace gl
{
    Fence::Fence() {}

    bool Fence::Create(const Device &device, uint32_t count)
    {
        handles.resize(count);

        for (uint32_t i = 0; i < count; ++i)
        {
            VkFenceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

            if (vkCreateFence(device.handle, &createInfo, nullptr, &handles[i]) != VK_SUCCESS)
            {
                fmtx::Error("Failed to create fence");
                return false;
            }
        }

        fmtx::Info("Created fences");

        return true;
    }

    void Fence::Destroy(const Device &device)
    {
        for (auto &handle : handles)
        {
            vkDestroyFence(device.handle, handle, nullptr);
        }
    }

    void Fence::Wait(int indexFence, const Device &device)
    {
        vkWaitForFences(device.handle, 1, &handles[indexFence], VK_TRUE, UINT64_MAX);
    }

    void Fence::Reset(int indexFence, const Device &device)
    {
        vkResetFences(device.handle, 1, &handles[indexFence]);
    }
}
