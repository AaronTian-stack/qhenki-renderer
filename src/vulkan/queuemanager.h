#pragma once

#include "swapchain.h"

class QueueManager
{
private:
    vk::Queue graphicsQueue;
    vk::Queue presentQueue;

public:
    QueueManager();

    void initQueues(vk::Device &device, VkStructs::QueueFamilyIndices indices);

    vk::Result submitGraphics(vk::SubmitInfo submitInfo, vk::Fence fence);
    vk::Result present(SwapChain &swapChain, const std::vector<vk::Semaphore> &signalSemaphores);

    friend class UserInterface;
    friend class CommandPool;
};
