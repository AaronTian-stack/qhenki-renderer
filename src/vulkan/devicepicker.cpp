#include <stdexcept>
#include <vector>
#include <set>
#include <iostream>
#include "devicepicker.h"
#include "swapchain.h"

DevicePicker::DevicePicker() {}

void DevicePicker::pickPhysicalDevice(VulkanInstance vkInstance, VkSurfaceKHR surface)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vkInstance.getInstance(), &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(vkInstance.getInstance(), &deviceCount, devices.data());

    VkPhysicalDeviceProperties deviceProperties;
    int i = 0;
    for (const auto &device : devices)
    {
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        bool isDiscrete = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

        if (isDeviceSuitable(device, surface))
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

        listQueueFamilies(device);
    }

    int deviceIndex = -1;
    std::cout << "Pick a device (type and enter a valid index): ";
    while (deviceIndex < 0 || deviceIndex >= deviceCount)
        std::cin >> deviceIndex;

    physicalDevice = devices[deviceIndex];

    if (physicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Failed to find a suitable GPU! (null handle)");
    }
}

QueueFamilyIndices DevicePicker::findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
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

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport)
        {
            indices.presentFamily = i;
        }

        if (indices.isComplete())
        {
            break;
        }

        i++;
    }

    return indices;
}

bool DevicePicker::isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    // can add more conditions in below function if wanted, such as geometry shader support
    QueueFamilyIndices indices = findQueueFamilies(device, surface);
    deviceToQueue[device] = indices;
    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = SwapChain::querySwapChainSupport(device, surface);
        // has some surface format and presentation mode
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    // does it have the queue families we wanted?
    // are the required extensions supported?
    // is the swap chain adequate?
    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

bool DevicePicker::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

void DevicePicker::createLogicalDevice()
{
    if (physicalDevice == VK_NULL_HANDLE)
        throw std::runtime_error("physical device is null");

    QueueFamilyIndices indices = deviceToQueue[physicalDevice];

    // create queue create info for each queue family
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        // describe number of queues we want for single queue family
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        // only need one because all command buffers sent to this queue can be multithreaded
        queueCreateInfo.queueCount = 1;
        // required even if only one queue
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // TODO: add device features
    // could just enable all available features by just calling vkGetPhysicalDeviceFeatures
    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;
    // enabledLayerCount and ppEnabledLayerNames are deprecated, no need to set them
    createInfo.enabledLayerCount = 0;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    // create logical device
    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device!");
    }
}

void DevicePicker::dispose()
{
    vkDestroyDevice(device, nullptr);
}

VkDevice DevicePicker::getDevice()
{
    return device;
}

QueueFamilyIndices DevicePicker::selectedDeviceFamily()
{
    return deviceToQueue[physicalDevice];
}

VkPhysicalDevice DevicePicker::getPhysicalDevice()
{
    return physicalDevice;
}

void DevicePicker::listQueueFamilies(VkPhysicalDevice device)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    // care about graphics, compute, and transfer, 1, 2, 4. compare against binary number
    std::cout << "Number of queue families: " << queueFamilyCount << std::endl;
    int i = 1;
    for (const auto &queueFamily : queueFamilies)
    {
        std::cout << "family " << i << ": " << queueFamily.queueFlags << queueFamily.queueCount << std::endl;
        i++;
    }
}

bool QueueFamilyIndices::isComplete()
{
    // found a graphics queue family (with graphics bit) and a present queue family
    return graphicsFamily.has_value() && presentFamily.has_value();;
}
