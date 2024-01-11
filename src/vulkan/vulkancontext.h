#pragma once

#include "instance.h"
#include "debugger.h"
#include "devicepicker.h"
#include "queuemanager.h"
#include "swapchain.h"

class VulkanContext : Disposable
{
public:
    VulkanInstance vulkanInstance;
    Debugger debugger;
    DevicePicker devicePicker;
    QueueManager queueManager;
    SwapChain swapChain;

    void create(Window &window);

    // DOES NOT DISPOSE VULKAN INSTANCE
    void dispose() override;
};
