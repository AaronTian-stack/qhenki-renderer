#pragma once

#include "smartpointer.h"
#include "vulkan/vkdebugger.h"
#include "vulkan/vkdevicepicker.h"
#include "vulkan/vkqueuemanager.h"
#include "vulkan/swapchainmanager.h"
#include "vulkan/vulkanpipeline.h"
#include "vulkan/vkpipelinebuilder.h"

class PathTracerApp : public Disposable
{
private:
    VulkanInstance vulkanInstance;
    VkDebugger vulkanDebugger;
    VkDevicePicker vulkanDevicePicker;
    VkQueueManager vulkanQueueManager;
    SwapChainManager swapChainManager;

    VulkanRenderPass renderPass;
    VkPipelineBuilder pipelineBuilder;
    uPtr<VulkanPipeline> pipeline;

public:
    PathTracerApp();
    ~PathTracerApp();

    void create(Window &window);
    void render();
    void resize();
    void dispose() override;

    VulkanInstance getVulkanInstance();
};
