#include "queuemanager.h"

QueueManager::QueueManager() {}

void QueueManager::initQueues(VkDevice device, QueueFamilyIndices indices)
{
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

