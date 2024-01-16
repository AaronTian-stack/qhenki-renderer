#pragma once

#include <optional>
#include <map>

#include "vulkan/vulkan.hpp"
#include "device.h"
#include "vkstructs.h"

// responsible for picking the physical device and creating the logical device
class DevicePicker
{
private:

    // required device extensions
#if __APPLE__
    const static inline std::vector<const char*> deviceExtensions =
    {
            "VK_KHR_portability_subset",
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
#else
    const static inline std::vector<const char*> deviceExtensions =
    {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
#endif

    static bool isDeviceSuitable(const vk::PhysicalDevice &device, VkSurfaceKHR surface);
    static bool checkDeviceExtensionSupport(const vk::PhysicalDevice &device);

    static vk::PhysicalDevice pickPhysicalDevice(vk::Instance &instance, VkSurfaceKHR surface);
    static std::pair<vk::Device, VkStructs::QueueFamilyIndices> createLogicalDevice(vk::PhysicalDevice &physicalDevice, VkSurfaceKHR surface);

public:
    static Device createDevice(vk::Instance &instance, VkSurfaceKHR surface);

    static VkStructs::QueueFamilyIndices findQueueFamilies(const vk::PhysicalDevice &device, VkSurfaceKHR surface);
    static void listQueueFamilies(const vk::PhysicalDevice &device);
};
