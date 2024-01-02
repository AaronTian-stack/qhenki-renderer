#pragma once

#include "smartpointer.h"
#include "vulkan/debugger.h"
#include "vulkan/devicepicker.h"
#include "vulkan/queuemanager.h"
#include "vulkan/swapchainmanager.h"
#include "vulkan/pipeline.h"
#include "vulkan/pipelinebuilder.h"
#include "vulkan/commandpool.h"
#include "vulkan/syncer.h"

class PathTracerApp : public Disposable
{
private:
    VulkanInstance vulkanInstance;
    Debugger debugger;
    DevicePicker devicePicker;
    QueueManager queueManager;
    SwapChainManager swapChainManager;

    RenderPass renderPass;
    PipelineBuilder pipelineBuilder;
    uPtr<Pipeline> pipeline;
    uPtr<Shader> shader;

    CommandPool commandPool; // one pool per thread
    Syncer syncer;

public:
    PathTracerApp();
    ~PathTracerApp();

    void create(Window &window);
    void render();
    void resize();
    void dispose() override;

    void recordCommandBuffer(uint32_t imageIndex); // TODO: NEED TO DELETE THIS LATER

    VulkanInstance getVulkanInstance();
};
