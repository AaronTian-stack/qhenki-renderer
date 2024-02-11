#pragma once

#include <vulkan/vulkan.hpp>
#include "vkstructs.h"

struct Device
{
    vk::PhysicalDevice physicalDevice;
    vk::Device logicalDevice;
    VkStructs::QueueFamilyIndices queueFamilyIndices;

    void destroy()
    {
        logicalDevice.destroy();
    }
};