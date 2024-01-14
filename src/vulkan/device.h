#pragma once

#include <vulkan/vulkan.hpp>
#include "queuefamilyindices.h"

struct Device
{
    vk::PhysicalDevice physicalDevice;
    vk::Device logicalDevice;
    QueueFamilyIndices queueFamilyIndices;

    void destroy()
    {
        logicalDevice.destroy();
    }
};