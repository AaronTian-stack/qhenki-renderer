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
#include <glm/ext.hpp>

class PathTracerApp
{
private:
    uPtr<Buffer> buffer;
    uPtr<Buffer> indexBuffer;

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
    std::vector<uPtr<Buffer>> cameraBuffers;

public:
    PathTracerApp();
    ~PathTracerApp();

    void create(Window &window);
    void render();
    void resize();

    void recordCommandBuffer(VkFramebuffer framebuffer); // TODO: NEED TO DELETE THIS LATER

    UserInterface *ui;
    ImGuiCreateParameters getImGuiCreateParameters();

    CommandPool& getCommandPool();
};
