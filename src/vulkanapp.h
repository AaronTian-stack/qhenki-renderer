#pragma once

#include "smartpointer.h"
#include "vulkan/queuemanager.h"
#include "vulkan/pipeline/pipeline.h"
#include "vulkan/pipeline/pipelinefactory.h"
#include "vulkan/commandpool.h"
#include "vulkan/syncer.h"
#include "vulkan/frame.h"
#include "userinterface.h"
#include "vulkan/context/vulkancontext.h"
#include "models/gltfloader.h"
#include "vulkan/buffer/bufferfactory.h"
#include "vulkan/descriptors/descriptorlayoutcache.h"
#include "vulkan/descriptors/descriptorallocator.h"
#include "vulkan/descriptors/descriptorbuilder.h"
#include <glm/ext.hpp>
#include "cameramatrices.h"
#include "camera.h"
#include "inputprocesser.h"
#include "vulkan/renderpass/renderpassbuilder.h"
#include "vulkan/texture/envmap.h"
#include "vulkan/attachments/gbuffer.h"
#include <atomic>
#include <mutex>

class VulkanApp
{
private:
    EnvironmentMap envMap;
    std::vector<uPtr<Model>> models;

    uPtr<GBuffer> gBuffer;
    sPtr<Attachment> depthBuffer;

    GLTFLoader gltfLoad;
    BufferFactory bufferFactory;

    VulkanContext vulkanContext;

    RenderPassBuilder renderPassBuilder;
    uPtr<RenderPass> displayRenderPass;
    uPtr<RenderPass> offscreenRenderPass;
    PipelineBuilder pipelineFactory;

    uPtr<Pipeline> gBufferPipeline, lightingPipeline, postProcessPipeline, cubeMapPipeline;

    uPtr<Shader> gBufferShader, lightingShader, postProcessShader, cubeMapShader;

    std::mutex modelMutex;
    uPtr<CommandPool> graphicsCommandPool; // one pool per thread
    uPtr<CommandPool> transferCommandPool;
    Syncer syncer;

    int currentFrame = 0;
    const int MAX_FRAMES_IN_FLIGHT = 2;
    std::vector<Frame> frames;
    std::vector<uPtr<Buffer>> cameraBuffers;

    Camera camera;
    CameraMatrices cameraMatrices;
    DescriptorLayoutCache layoutCache;
    std::vector<DescriptorAllocator> allocators;

public:
    VulkanApp();
    ~VulkanApp();

    void createGbuffer(vk::Extent2D extent, vk::RenderPass renderPass);
    void create(Window &window);
    void render();
    void resize();

    void handleInput();
    void updateCameraBuffer();
    void recordOffscreenBuffer(vk::CommandBuffer buffer, DescriptorAllocator &allocator);
    void recordCommandBuffer(vk::Framebuffer framebuffer); // TODO: NEED TO DELETE THIS LATER

    UserInterface *ui;
    ImGuiCreateParameters getImGuiCreateParameters();

    CommandPool& getGraphicsCommandPool();

    void setUpCallbacks();
    void setModel(const std::string &filePath);
};
