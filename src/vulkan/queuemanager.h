#pragma once

#include "context/swapchain.h"
#include "../smartpointer.h"
#include "context/queuesindices.h"

class QueueManager
{
private:
    QueuesIndices queuesIndices;

public:
    QueueManager();

    void initQueues(QueuesIndices queuesIndices);

    vk::Result submitGraphics(vk::SubmitInfo submitInfo, vk::Fence fence);
    vk::Result present(const uPtr<SwapChain> &swapChain, const std::vector<vk::Semaphore> &signalSemaphores);

    uint32_t getGraphicsIndex() { return queuesIndices.graphicsIndex; }
    uint32_t getPresentIndex() { return queuesIndices.presentIndex; }
    uint32_t getTransferIndex() { return queuesIndices.transferIndex; }

    friend class UserInterface;
    friend class CommandPool;
};
