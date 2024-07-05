#pragma once

#include "../../window.h"
#include "../queuemanager.h"
#include <VkBootstrap.h>
#include <smartpointer.h>
#include "queuesindices.h"

struct Device
{
    vkb::Device vkbDevice;
    vk::PhysicalDevice physicalDevice;
    vk::Device logicalDevice;
};

class VulkanContext
{
private:
    Window *window = nullptr;
    QueuesIndices selectQueues(vkb::Device &vkb_device, vk::Device device);

public:
    vkb::Instance vkbInstance;

    Device device;

    QueueManager queueManager;
    uPtr<SwapChain> swapChain;

    bool create(Window &window);
    ~VulkanContext();
};
