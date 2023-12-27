#pragma once

#include "smartpointer.h"
#include "vulkan/vkdebugger.h"
#include "vulkan/vkdevicepicker.h"
#include "vulkan/vkqueuemanager.h"
#include "vulkan/swapchainmanager.h"

class PathTracerApp
{
private:
    VulkanInstance vulkanInstance;
    VkDebugger vulkanDebugger;
    VkDevicePicker vulkanDevicePicker;
    VkQueueManager vulkanQueueManager;
    SwapChainManager swapChainManager;

public:
    PathTracerApp();
    ~PathTracerApp();

    void create(Window &window);
    void render();
    void resize();
    void destroy();

    VulkanInstance getVulkanInstance();
};
