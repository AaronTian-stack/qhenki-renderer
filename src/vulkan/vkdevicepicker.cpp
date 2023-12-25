#include <stdexcept>
#include <vector>
#include <iostream>
#include "vkdevicepicker.h"

VkDevicePicker::VkDevicePicker() {}

void VkDevicePicker::pickPhysicalDevice(VulkanInstance vkInstance)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vkInstance.instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(vkInstance.instance, &deviceCount, devices.data());

    VkPhysicalDeviceProperties deviceProperties;
    int i = 0;
    for (const auto &device : devices)
    {
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        bool isDiscrete = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

        if (isDeviceSuitable(device))
        {
            std::cout << "(" << i << ") ";
            i++;
        }
        else
        {
            std::cout << "(INVALID) ";
        }

        isDiscrete ?
        std::cout << "Device " << deviceProperties.deviceName << ": Discrete GPU\n"
        : std::cout << "Device " << deviceProperties.deviceName << ": Integrated GPU\n";
    }

    int deviceIndex = -1;
    std::cout << "Pick a device (valid index): ";
    while (deviceIndex < 0 || deviceIndex >= deviceCount)
        std::cin >> deviceIndex;

    physicalDevice = devices[deviceIndex];

    if (physicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Failed to find a suitable GPU! (null handle)");
    }
}

QueueFamilyIndices VkDevicePicker::findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    // get number of queue families
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    // put queue families into vector
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    // find queue family with graphics bit
    int i = 0;
    for (const auto &queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }
        if (indices.isComplete())
        {
            break;
        }
        i++;
    }

    return indices;
}

bool VkDevicePicker::isDeviceSuitable(VkPhysicalDevice device)
{
    // can add more conditions if wanted, such as geometry shader support
    QueueFamilyIndices indices = findQueueFamilies(device);
    // does it have the queue families we wanted?
    return indices.isComplete();
}

bool QueueFamilyIndices::isComplete()
{
    // found a graphics queue family (with graphics bit)
    return graphicsFamily.has_value();
}
