#pragma once

#include "smartpointer.h"
#include "vulkan/queuemanager.h"
#include "vulkan/pipeline/pipeline.h"
#include "vulkan/pipeline/pipelinefactory.h"
#include "vulkan/commandpool.h"
#include "vulkan/syncer.h"
#include "vulkan/frame.h"
#include "ui/userinterface.h"
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
#include "vfx/postprocessmanager.h"
#include "vfx/effects/fxaa.h"
#include "lights/lights.h"
#include <atomic>
#include <mutex>

class VulkanApp
{
private:
    EnvironmentMap envMap;
    std::vector<uPtr<Model>> models;

    LightingParameters lightingParameters;
    uPtr<GBuffer> gBuffer;
    uPtr<Attachment> depthBuffer;

    GLTFLoader gltfLoad;
    BufferFactory bufferFactory;

    VulkanContext vulkanContext;

    RenderPassBuilder renderPassBuilder;
    uPtr<RenderPass> displayRenderPass;
    uPtr<RenderPass> offscreenRenderPass;
    PipelineBuilder pipelineFactory;

    uPtr<Pipeline> gBufferPipeline, lightingPipeline, passPipeline, passAndClearPipeline, cubeMapPipeline, lightDisplayPipeline;

    uPtr<Shader> gBufferShader, lightingShader, passShader, passAndClearShader, cubeMapShader, lightDisplayShader;

    std::mutex modelMutex;
    uPtr<CommandPool> graphicsCommandPool; // one pool per thread
    uPtr<CommandPool> transferCommandPool;
    Syncer syncer;

    int currentFrame = 0;
    const int MAX_FRAMES_IN_FLIGHT = 2;
    std::vector<Frame> frames;

    std::vector<uPtr<Buffer>> cameraBuffers;

    const int MAX_LIGHTS = 10;
//    std::vector<uPtr<Buffer>> pointLightBuffers;
    std::vector<SphereLight> sphereLights;
    std::vector<uPtr<Buffer>> sphereLightBuffers;
    std::vector<TubeLight> tubeLights;
    std::vector<uPtr<Buffer>> tubeLightsBuffers;
//    std::vector<uPtr<Buffer>> rectangleLightBuffers;

    Camera camera;
    CameraMatrices cameraMatrices;
    DescriptorLayoutCache layoutCache;
    std::vector<DescriptorAllocator> allocators;

public:
    VulkanApp();
    ~VulkanApp();

    void createGbuffer(vk::Extent2D extent, vk::RenderPass renderPass);
    void createRenderPasses();
    void createPipelines();
    void createPostProcess();
    void create(Window &window);
    void render();
    void resize();

    void handleInput();
    void updateLightBuffers();
    void updateCameraBuffer();

    void attemptToDeleteOldModel();

    void recordOffscreenBuffer(vk::CommandBuffer buffer, DescriptorAllocator &allocator);
    void recordCommandBuffer(FrameBuffer *framebuffer);

    uPtr<PostProcessManager> postProcessManager;
    UserInterface *ui;
    ImGuiCreateParameters getImGuiCreateParameters();

    CommandPool& getGraphicsCommandPool();

    void setUpCallbacks();
    void setModel(const std::string &filePath);
};
