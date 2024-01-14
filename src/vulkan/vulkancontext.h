#pragma once

#include "../smartpointer.h"
#include "../window.h"
#include "queuemanager.h"
#include "swapchain.h"
#include "device.h"

class VulkanContext
{
private:
    Window *window = nullptr;

public:
    vk::Instance instance;
    vk::DebugUtilsMessengerEXT debugger;

    Device device;

    QueueManager queueManager;
    SwapChain swapChain;

    void create(Window &window);
    ~VulkanContext();
};
