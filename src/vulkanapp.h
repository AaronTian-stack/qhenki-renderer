#pragma once

#include <smartpointer.h>
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
#include "vulkan/pipeline/pipelineshader.h"
#include <atomic>
#include <mutex>

class VulkanApp
{
private:
    bool drawBackground, drawGrid;
    glm::vec3 clearColor;
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

    PipelineShader gBufferPS, lighting, pass, passAndClear, cubeMap, lightDisplay, solidPlane, gridPlane;
    PipelineShader skinning;

    std::mutex modelMutex;
    uPtr<CommandPool> graphicsCommandPool; // one pool per thread
    uPtr<CommandPool> transferCommandPool;
    Syncer syncer;

    int currentFrame = 0;
    const int MAX_FRAMES_IN_FLIGHT = 2;
    std::vector<Frame> frames;
    std::vector<vk::CommandBuffer> computeCommandBuffers;
    std::vector<vk::Semaphore> computeSemaphores;
    std::vector<vk::Fence> computeFences;

    uPtr<Buffer> bayerMatrix;
    std::vector<uPtr<Buffer>> cameraBuffers;

    const int MAX_LIGHTS = 10;
//    std::vector<uPtr<Buffer>> pointLightBuffers;
    std::vector<SphereLight> sphereLights;
    std::vector<uPtr<Buffer>> sphereLightBuffers;
    std::vector<TubeLight> tubeLights;
    std::vector<uPtr<Buffer>> tubeLightsBuffers;
    std::vector<RectangleLight> rectangleLights;
    std::vector<uPtr<Buffer>> rectangleLightBuffers;

    uPtr<PostProcessManager> postProcessManager;

    float cubeMapRotation;
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

    UserInterface *ui;
    ImGuiCreateParameters getImGuiCreateParameters();
    MenuPayloads getPartialMenuPayload();

    CommandPool& getGraphicsCommandPool();

    void setUpCallbacks();
    void setModel(const std::string &filePath);
};
