#pragma once

#include <optional>
#include <map>

#include "vulkan/vulkan.h"
#include "vulkaninstance.h"

struct QueueFamilyIndices
{
    // for drawing
    std::optional<uint32_t> graphicsFamily;
    // for presentation
    std::optional<uint32_t> presentFamily;

    bool isComplete();
};

// responsible for picking the physical device and creating the logical device
class VkDevicePicker
{
private:

    // required device extensions
    const std::vector<const char*> deviceExtensions =
    {
            "VK_KHR_portability_subset",
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    // note: theoretically, we could have multiple physical devices and run different operations on them
    // VkPhysicalDevice is implicitly destroyed with VkInstance
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    // logical device needs to be explicitly destroyed
    VkDevice device;
    bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    // check if the device has the queue families we need
    std::unordered_map<VkPhysicalDevice, QueueFamilyIndices> deviceToQueue;

public:
    VkDevicePicker();
    void pickPhysicalDevice(VulkanInstance vkInstance, VkSurfaceKHR surface);
    QueueFamilyIndices selectedDeviceFamily();
    // creates a logical device from physicalDevice
    void createLogicalDevice();
    void destroy();

    VkDevice getDevice();
    VkPhysicalDevice getPhysicalDevice();

    static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
};
