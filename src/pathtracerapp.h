#pragma once

#include "smartpointer.h"
#include "vulkan/debugger.h"
#include "vulkan/devicepicker.h"
#include "vulkan/queuemanager.h"
#include "vulkan/swapchain.h"
#include "vulkan/pipeline.h"
#include "vulkan/pipelinefactory.h"
#include "vulkan/commandpool.h"
#include "vulkan/syncer.h"
#include "vulkan/frame.h"
#include "userinterface.h"
#include "vulkan/vulkancontext.h"
#include "gltfloader.h"
#include "vulkan/bufferfactory.h"

class PathTracerApp : public Disposable
{
private:
    uPtr<Buffer> buffer;

    GLTFLoader gltfLoad;
    BufferFactory bufferFactory;

    VulkanContext vulkanContext;

    RenderPass renderPass;
    PipelineBuilder pipelineFactory;

    uPtr<Pipeline> pipeline1, pipeline2;
    uPtr<Shader> shader1, shader2;

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

    UserInterface *ui;
    VulkanInstance& getVulkanInstance();
    ImGuiCreateParameters getImGuiCreateParameters();

    CommandPool& getCommandPool();
};
