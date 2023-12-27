#include "vulkan/vulkan.h"
#include "vkdevicepicker.h"

class VkQueueManager
{
private:
    VkQueue graphicsQueue;
    VkQueue presentQueue;

public:
    VkQueueManager();

    /*
     * Initializes the queues for the device.
     * @param device The device to get the queues from.
     * @param indices The indices of the queue families corresponding to the device you are using.
     */
    void initQueues(VkDevice device, QueueFamilyIndices indices);

    //VkQueue getGraphicsQueue() const;
    //VkQueue getPresentQueue() const;

};
