#include "vulkan/vulkan.h"
#include "vkdevicepicker.h"

class VkQueueManager
{
private:
    VkQueue graphicsQueue;
    VkQueue presentQueue;

public:
    VkQueueManager();
    void initQueues(VkDevice device, QueueFamilyIndices indices);
    //VkQueue getGraphicsQueue() const;
    //VkQueue getPresentQueue() const;

};
