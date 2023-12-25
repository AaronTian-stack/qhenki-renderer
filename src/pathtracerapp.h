#pragma once

#include "application.h"
#include "window.h"
#include "smartpointer.h"
#include "vulkan/vkdebugger.h"
#include "vulkan/vkdevicepicker.h"

class PathTracerApp : public Application
{
private:
    VulkanInstance vulkanInstance;
    VkDebugger vulkanDebugger;
    VkDevicePicker vulkanDevicePicker;

public:
    PathTracerApp();
    ~PathTracerApp();
    void create() override;
    void render() override;
    void resize() override;
};
