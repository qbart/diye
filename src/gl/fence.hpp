#pragma once

#include "core.hpp"
#include "device.hpp"

namespace gl
{
class Fence
{
public:
    std::vector<VkFence> handles;

    Fence();
    bool Create(const Device &device, uint32_t count);
    void Destroy(const Device &device);
    void Wait(int indexFence, const Device &device);
    void Reset(int indexFence, const Device &device);
};
} // namespace gl