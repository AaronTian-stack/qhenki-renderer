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

    // TODO: delete these getters
    //VkQueue getGraphicsQueue() const { return graphicsQueue;}
    //VkQueue getPresentQueue() const { return presentQueue; }

    void submitGraphics(VkSubmitInfo submitInfo, VkFence fence);
    void present(SwapChain &swapChain, const std::vector<VkSemaphore> &signalSemaphores);

};
