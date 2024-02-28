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
#include "gltfloader.h"
#include "vulkan/bufferfactory.h"
#include "vulkan/descriptors/descriptorlayoutcache.h"
#include "vulkan/descriptors/descriptorallocator.h"
#include "vulkan/descriptors/descriptorbuilder.h"
#include <glm/ext.hpp>
#include "cameramatrices.h"
#include "camera.h"
#include "inputprocesser.h"

class VulkanApp
{
private:
    //uPtr<Buffer> buffer;
    //uPtr<Buffer> colorBuffer;
    //uPtr<Buffer> indexBuffer;

    GLTFLoader gltfLoad;
    BufferFactory bufferFactory;

    VulkanContext vulkanContext;

    RenderPass renderPass;
    PipelineBuilder pipelineFactory;

    uPtr<Pipeline> pathPipeline;//, triPipeline;
    uPtr<Shader> shader1, shader2;

    CommandPool commandPool; // one pool per thread
    Syncer syncer;

    int currentFrame = 0;
    const int MAX_FRAMES_IN_FLIGHT = 2;
    std::vector<Frame> frames;
    std::vector<uPtr<Buffer>> cameraBuffers;

    Camera camera;
    CameraMatrices cameraMatrices;
    DescriptorLayoutCache layoutCache;
    std::vector<DescriptorAllocator> allocators;

    glm::mat4 modelTransform;

public:
    VulkanApp();
    ~VulkanApp();

    void create(Window &window);
    void render();
    void resize();

    void handleInput();
    void updateCameraBuffer();
    void recordCommandBuffer(VkFramebuffer framebuffer); // TODO: NEED TO DELETE THIS LATER

    UserInterface *ui;
    ImGuiCreateParameters getImGuiCreateParameters();

    CommandPool& getCommandPool();
};
