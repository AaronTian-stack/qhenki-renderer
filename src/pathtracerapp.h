#pragma once

#include "window.h"
#include "smartpointer.h"
#include "vulkan/vkdebugger.h"
#include "vulkan/vkdevicepicker.h"
#include "vulkan/vkqueuemanager.h"

class PathTracerApp
{
private:
    VulkanInstance vulkanInstance;
    VkDebugger vulkanDebugger;
    VkDevicePicker vulkanDevicePicker;
    VkQueueManager vulkanQueueManager;

public:
    PathTracerApp();
    ~PathTracerApp();
    void create(Window &window);
    void render();
    void resize();

    VulkanInstance getVulkanInstance();
};
