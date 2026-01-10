#pragma once

#include "core.hpp"

namespace gl
{
class Queue
{
public:
    VkQueue handle;
    VkSubmitInfo submitInfo;
    VkPresentInfoKHR presentInfo;
    std::vector<VkPipelineStageFlags> waitStages;
    std::vector<VkSemaphore> waitSemaphores;
    std::vector<VkSemaphore> signalSemaphores;
    std::vector<uint32_t> imageIndices;
    std::vector<VkSwapchainKHR> swapChains;

    Queue(VkQueue handle);

    void Clear();
    void AddWaitSemaphore(VkSemaphore semaphore);
    void AddSignalSemaphore(VkSemaphore semaphore);
    void AddWaitStage(VkPipelineStageFlags stage);
    void AddSwapChain(VkSwapchainKHR swapChain);
    void AddImageIndex(uint32_t imageIndex);
    VkResult Submit(VkCommandBuffer commandBuffer, VkFence fence = VK_NULL_HANDLE);
    VkResult Present();
};
} // namespace gl