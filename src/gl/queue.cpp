#include "queue.hpp"

namespace gl
{
    Queue::Queue(VkQueue handle) : handle(handle),
                                   submitInfo({}),
                                   presentInfo({})
    {
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    }

    void Queue::Clear()
    {
        waitSemaphores.clear();
        signalSemaphores.clear();
        waitStages.clear();

        imageIndices.clear();
        swapChains.clear();
    }

    void Queue::AddWaitSemaphore(VkSemaphore semaphore)
    {
        waitSemaphores.push_back(semaphore);
    }

    void Queue::AddSignalSemaphore(VkSemaphore semaphore)
    {
        signalSemaphores.push_back(semaphore);
    }

    void Queue::AddWaitStage(VkPipelineStageFlags stage)
    {
        waitStages.push_back(stage);
    }

    void Queue::AddSwapChain(VkSwapchainKHR swapChain)
    {
        swapChains.push_back(swapChain);
    }

    void Queue::AddImageIndex(uint32_t imageIndex)
    {
        imageIndices.push_back(imageIndex);
    }

    VkResult Queue::Submit(VkCommandBuffer commandBuffer, VkFence fence)
    {
        submitInfo.waitSemaphoreCount = waitSemaphores.size();
        submitInfo.pWaitSemaphores = waitSemaphores.data();
        submitInfo.pWaitDstStageMask = waitStages.data();

        submitInfo.signalSemaphoreCount = signalSemaphores.size();
        submitInfo.pSignalSemaphores = signalSemaphores.data();

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        return vkQueueSubmit(handle, 1, &submitInfo, fence);
    }

    VkResult Queue::Present()
    {
        presentInfo.waitSemaphoreCount = waitSemaphores.size();
        presentInfo.pWaitSemaphores = waitSemaphores.data();

        presentInfo.swapchainCount = swapChains.size();
        presentInfo.pSwapchains = swapChains.data();

        presentInfo.pImageIndices = imageIndices.data();
        presentInfo.pResults = nullptr;

        return vkQueuePresentKHR(handle, &presentInfo);
    }
}
