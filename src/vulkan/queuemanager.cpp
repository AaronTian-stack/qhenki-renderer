#include "queuemanager.h"

QueueManager::QueueManager() {}

void QueueManager::initQueues(vk::Device& device, QueueFamilyIndices indices)
{
    // since one queue per family just get the first queue
    graphicsQueue = device.getQueue(indices.graphicsFamily.value(), 0);
    presentQueue = device.getQueue(indices.presentFamily.value(), 0);
}

vk::Result QueueManager::submitGraphics(vk::SubmitInfo submitInfo, vk::Fence fence)
{
    // see vkQueueSubmit
    return graphicsQueue.submit(1, &submitInfo, fence);
}

vk::Result QueueManager::present(SwapChain &swapChain, const std::vector<vk::Semaphore> &signalSemaphores)
{
    auto presentInfo = vk::PresentInfoKHR(
        signalSemaphores.size(),
        signalSemaphores.data(), // wait for this semaphore to be signaled before presentation
        1,
        &swapChain.swapChain,
        &swapChain.imageIndex
    );

    return presentQueue.presentKHR(presentInfo);
}

