#pragma once

#include "context/swapchain.h"
#include "../smartpointer.h"

class QueueManager
{
private:
    vk::Queue graphicsQueue;
    vk::Queue presentQueue;

public:
    QueueManager();

    void initQueues(vk::Queue graphicsQueue, vk::Queue presentQueue);

    vk::Result submitGraphics(vk::SubmitInfo submitInfo, vk::Fence fence);
    vk::Result present(const uPtr<SwapChain> &swapChain, const std::vector<vk::Semaphore> &signalSemaphores);

    friend class UserInterface;
    friend class CommandPool;
};
