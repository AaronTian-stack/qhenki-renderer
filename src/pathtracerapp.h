#pragma once

#include "smartpointer.h"
#include "vulkan/debugger.h"
#include "vulkan/devicepicker.h"
#include "vulkan/queuemanager.h"
#include "vulkan/swapchain.h"
#include "vulkan/pipeline.h"
#include "vulkan/pipelinebuilder.h"
#include "vulkan/commandpool.h"
#include "vulkan/syncer.h"
#include "vulkan/frame.h"

class PathTracerApp : public Disposable
{
private:
    VulkanInstance vulkanInstance;
    Debugger debugger;
    DevicePicker devicePicker;
    QueueManager queueManager;
    SwapChain swapChain;

    RenderPass renderPass;
    PipelineBuilder pipelineBuilder;
    uPtr<Pipeline> pipeline;
    uPtr<Shader> shader;

    CommandPool commandPool; // one pool per thread
    Syncer syncer;

    int currentFrame = 0;
    const int MAX_FRAMES_IN_FLIGHT = 2;
    std::vector<Frame> frames;

public:
    PathTracerApp();
    ~PathTracerApp();

    void create(Window &window);
    void render();
    void resize();
    void dispose() override;

    void recordCommandBuffer(VkFramebuffer framebuffer); // TODO: NEED TO DELETE THIS LATER

    VulkanInstance getVulkanInstance();
};
