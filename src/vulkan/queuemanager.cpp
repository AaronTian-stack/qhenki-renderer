#include "queuemanager.h"

QueueManager::QueueManager() {}

void QueueManager::initQueues(QueuesIndices queuesIndices)
{
    this->queuesIndices = queuesIndices;
}

vk::Result QueueManager::submitGraphics(vk::SubmitInfo submitInfo, vk::Fence fence)
{
    // see vkQueueSubmit
    return queuesIndices.graphics.submit(1, &submitInfo, fence);
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

    return queuesIndices.present.presentKHR(presentInfo);
}

