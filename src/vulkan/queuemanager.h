#pragma once

#include "vulkan/vulkan.h"
#include "devicepicker.h"
#include "swapchain.h"

class QueueManager
{
private:
    VkQueue graphicsQueue;
    VkQueue presentQueue;

public:
    QueueManager();

    /*
     * Initializes the queues for the device.
     * @param device The device to get the queues from.
     * @param indices The indices of the queue families corresponding to the device you are using.
     */
    void initQueues(VkDevice device, QueueFamilyIndices indices);

    void submitGraphics(VkSubmitInfo submitInfo, VkFence fence);
    void present(SwapChain &swapChain, const std::vector<VkSemaphore> &signalSemaphores);

    friend class UserInterface;
    friend class CommandPool;
};
