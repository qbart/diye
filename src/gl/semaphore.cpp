#include "semaphore.hpp"

namespace gl
{
    Semaphore::Semaphore() {}

    bool Semaphore::Create(const Device &device, uint32_t count)
    {
        handles.resize(count);

        for (uint32_t i = 0; i < count; ++i)
        {
            VkSemaphoreCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            if (vkCreateSemaphore(device.handle, &createInfo, nullptr, &handles[i]) != VK_SUCCESS)
            {
                fmtx::Error("Failed to create semaphore");
                return false;
            }
        }

        fmtx::Info("Created semaphores");

        return true;
    }

    void Semaphore::Destroy(const Device &device)
    {
        for (auto &handle : handles)
        {
            vkDestroySemaphore(device.handle, handle, nullptr);
        }

        handles.clear();
    }
}
