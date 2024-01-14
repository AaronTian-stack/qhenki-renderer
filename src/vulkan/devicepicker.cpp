#include <stdexcept>
#include <vector>
#include <set>
#include <iostream>
#include "devicepicker.h"
#include "swapchain.h"


vk::PhysicalDevice DevicePicker::pickPhysicalDevice(vk::Instance &instance, VkSurfaceKHR surface)
{
    auto devices = instance.enumeratePhysicalDevices();
    if (devices.empty()) throw std::runtime_error("Failed to find GPUs with Vulkan support!");

    int i = 0;
    for (const auto &device : devices)
    {
        auto deviceProperties = device.getProperties();

        bool isDiscrete = deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu;

        if (isDeviceSuitable(device, surface))
        {
            std::cout << "(" << i << ") ";
            i++;
        }
        else
        {
            std::cout << "(INVALID) ";
        }

        std::cout << "Device: ";

        isDiscrete ?
        std::cout << deviceProperties.deviceName << " Discrete GPU\n"
        : std::cout << deviceProperties.deviceName << " Integrated GPU\n";

        listQueueFamilies(device);
    }

    // TODO: needs to stop you from choosing unsupported device
    int deviceIndex = 0;
    if (devices.size() > 1)
    {
        deviceIndex = -1;
        std::cout << "Pick a device (type and enter a valid index): \n";
        while (deviceIndex < 0 || deviceIndex >= devices.size())
            std::cin >> deviceIndex;
    }

    return devices[deviceIndex];
}

bool DevicePicker::isDeviceSuitable(const vk::PhysicalDevice &device, VkSurfaceKHR surface)
{
    // can add more conditions in below function if wanted, such as geometry shader support
    QueueFamilyIndices indices = findQueueFamilies(device, surface);
    //deviceToQueue[device] = indices;
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

QueueFamilyIndices DevicePicker::findQueueFamilies(const vk::PhysicalDevice &device, VkSurfaceKHR surface)
{
    QueueFamilyIndices indices;

    auto queueFamilies = device.getQueueFamilyProperties();

    // find queue family with graphics bit
    int i = 0;
    for (const auto &queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
        {
            indices.graphicsFamily = i;
        }

        auto presentSupport = device.getSurfaceSupportKHR(i, surface);

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

bool DevicePicker::checkDeviceExtensionSupport(const vk::PhysicalDevice &device)
{
    auto availableExtensions = device.enumerateDeviceExtensionProperties();

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

std::pair<vk::Device, QueueFamilyIndices> DevicePicker::createLogicalDevice(vk::PhysicalDevice &physicalDevice, VkSurfaceKHR surface)
{
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

    // create queue create info for each queue family
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
    float queuePriority = 1.0f;
    queueCreateInfos.reserve(uniqueQueueFamilies.size());
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        // describe number of queues we want for single queue family
        queueCreateInfos.emplace_back(
                vk::DeviceQueueCreateFlags(),
                queueFamily,
                1, // only need one because all command buffers sent to this queue can be multithreaded
                &queuePriority); // required even if only one queue
    }

    // could just enable all available features by just calling vkGetPhysicalDeviceFeatures
    vk::PhysicalDeviceFeatures deviceFeatures{};

    vk::DeviceCreateInfo createInfo = vk::DeviceCreateInfo(
            vk::DeviceCreateFlags(),
            static_cast<uint32_t>(queueCreateInfos.size()),
            queueCreateInfos.data(),
            0, // deprecated
            nullptr, // deprecated
            static_cast<uint32_t>(deviceExtensions.size()),
            deviceExtensions.data(),
            &deviceFeatures
    );

    return {physicalDevice.createDevice(createInfo), indices};
}

void DevicePicker::listQueueFamilies(const vk::PhysicalDevice &device)
{
    auto queueFamilies = device.getQueueFamilyProperties();

    // care about graphics, compute, and transfer, 1, 2, 4. compare against binary number
    std::cout << "Number of queue families: " << queueFamilies.size() << std::endl;
    int i = 0;
    for (const auto &queueFamily : queueFamilies)
    {
        std::cout << "\tfamily " << i << ": " << static_cast<uint>(queueFamily.queueFlags) << static_cast<uint>(queueFamily.queueCount) << std::endl;
        i++;
    }
}

Device DevicePicker::createDevice(vk::Instance &instance, VkSurfaceKHR surface)
{
    auto physicalDevice = pickPhysicalDevice(instance, surface);
    auto create = createLogicalDevice(physicalDevice, surface);
    return { physicalDevice, create.first, create.second };
}
