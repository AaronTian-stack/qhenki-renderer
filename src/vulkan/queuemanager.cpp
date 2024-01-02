#include "queuemanager.h"

QueueManager::QueueManager() {}

void QueueManager::initQueues(VkDevice device, QueueFamilyIndices indices)
{
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

void QueueManager::submitGraphics(VkSubmitInfo submitInfo, VkFence fence)
{
    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, fence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }
}

void QueueManager::present(SwapChain &swapChain, const std::vector<VkSemaphore> &signalSemaphores)
{
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = signalSemaphores.size();
    presentInfo.pWaitSemaphores = signalSemaphores.data(); // wait for this semaphore to be signaled before presentation

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapChain.swapChain;
    presentInfo.pImageIndices = &swapChain.imageIndex;

    vkQueuePresentKHR(presentQueue, &presentInfo);
}

