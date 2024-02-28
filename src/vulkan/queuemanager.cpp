#include "queuemanager.h"

QueueManager::QueueManager() {}

void QueueManager::initQueues(vk::Queue graphicsQueue, vk::Queue presentQueue)
{
    this->graphicsQueue = graphicsQueue;
    this->presentQueue = presentQueue;
}

vk::Result QueueManager::submitGraphics(vk::SubmitInfo submitInfo, vk::Fence fence)
{
    // see vkQueueSubmit
    return graphicsQueue.submit(1, &submitInfo, fence);
}

vk::Result QueueManager::present(const uPtr<SwapChain> &swapChain, const std::vector<vk::Semaphore> &signalSemaphores)
{
    auto presentInfo = vk::PresentInfoKHR(
        signalSemaphores.size(),
        signalSemaphores.data(), // wait for this semaphore to be signaled before presentation
        1,
        &swapChain->swapChain,
        &swapChain->imageIndex
    );

    return presentQueue.presentKHR(presentInfo);
}

