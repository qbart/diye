#pragma once

#include "core.hpp"
#include "device.hpp"

namespace gl
{
class Semaphore
{
public:
    std::vector<VkSemaphore> handles;

    Semaphore();
    bool Create(const Device &device, uint32_t count);
    void Destroy(const Device &device);
};
} // namespace gl