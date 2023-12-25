#pragma once

#include <optional>

#include "vulkan/vulkan.h"
#include "vulkaninstance.h"

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;

    bool isComplete();
};

class VkDevicePicker
{

private:
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    static bool isDeviceSuitable(VkPhysicalDevice device);
    // check if the device has the queue families we need
    static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

public:
    VkDevicePicker();
    void pickPhysicalDevice(VulkanInstance vkInstance);

};
