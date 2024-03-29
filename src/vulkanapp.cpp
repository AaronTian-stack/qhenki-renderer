#include "vulkanapp.h"
#include "vulkan/screenutils.h"
#include "glm/glm.hpp"
#include "inputprocesser.h"
#include "imgui/imgui.h"

VulkanApp::VulkanApp() : camera({0, 2.f, 2.f}) {}

VulkanApp::~VulkanApp()
{
    vulkanContext.device.logicalDevice.waitIdle(); // wait for operations to finish before disposing
    
    for (auto &frame : frames)
        frame.destroy();

    syncer.destroy();
    commandPool.destroy();

    pathPipeline->destroy();
    shader1->destroy(); // shader modules must be destroyed after pipeline

    triPipeline->destroy();
    shader2->destroy();

    pipelineFactory.destroy();

    renderPass->destroy();
    renderPassBuilder.destroy();

    depthBuffer->destroy();

    model->destroy();

    texture->destroy();

    //buffer->destroy();
    positionBuffer->destroy();
    normalBuffer->destroy();
    indexBuffer->destroy();

    for (auto &cameraBuffer : cameraBuffers)
        cameraBuffer->destroy();

    for (auto &allocator : allocators)
        allocator.destroy();

    layoutCache.destroy();
    bufferFactory.destroy();
}

void VulkanApp::create(Window &window)
{
    InputProcesser::setCallbacks(window);

    vulkanContext.create(window);

    layoutCache.create(vulkanContext.device.logicalDevice);
    allocators.emplace_back(vulkanContext.device.logicalDevice);

    auto device = vulkanContext.device.logicalDevice;
    commandPool.create(vulkanContext.device);
    pipelineFactory.create(device);

    bufferFactory.create(vulkanContext);
    auto extent = vulkanContext.swapChain->getExtent();
    depthBuffer = bufferFactory.createAttachment(vk::Format::eD32Sfloat, {extent.width, extent.height, 1},
                                                   vk::ImageUsageFlagBits::eDepthStencilAttachment,
                                                   vk::ImageAspectFlagBits::eDepth);

    renderPassBuilder.create(vulkanContext.device.logicalDevice);

    // create render pass
    renderPassBuilder.addColorAttachment(vulkanContext.swapChain->getFormat());
    renderPassBuilder.addDepthAttachment(depthBuffer->format);
    renderPassBuilder.addSubPass({0}, 1);
    renderPass = renderPassBuilder.buildRenderPass();

    shader1 = mkU<Shader>(device, "pathtrace_vert.spv", "pathtrace_frag.spv");
    shader2 = mkU<Shader>(device, "tri_vert.spv", "tri_frag.spv");

    pipelineFactory.parseShader("pathtrace_vert.spv", "pathtrace_frag.spv", layoutCache, false);

    pathPipeline = pipelineFactory.buildPipeline(renderPass.get(), shader1.get());

    //// VERTEX INPUT
    const std::vector<glm::vec3> positions = {
            {-0.5f, -0.5f, 1.f},
            {0.5f, -0.5f, 0.f},
            {0.5f, 0.5f, 0.f},
            {-0.5f, 0.5f, 0.f}
    };
    const std::vector<glm::vec3> normals = {
            {0.0f, 0.0f, 1.0f},
            {0.0f, 0.0f, 1.0f},
            {0.0f, 0.0f, 1.0f},
            {0.0f, 0.0f, 1.0f}
    };
    const std::vector<uint16_t> indices = {
            0, 1, 2, 2, 3, 0
    };

    /*auto stagingBuffer = bufferFactory.createBuffer(
            vertices.size() * sizeof(Vertex),
            vk::BufferUsageFlagBits::eTransferSrc,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

    stagingBuffer->fill(vertices.data());

    buffer = bufferFactory.createBuffer(vertices.size() * sizeof(Vertex),
                                        vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst);
    stagingBuffer->copyTo(*buffer, vulkanContext.queueManager, commandPool);
    stagingBuffer->destroy();*/

    positionBuffer = bufferFactory.createBuffer(positions.size() * sizeof(glm::vec3), vk::BufferUsageFlagBits::eVertexBuffer, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
    positionBuffer->fill(positions.data());

    normalBuffer = bufferFactory.createBuffer(normals.size() * sizeof(glm::vec3), vk::BufferUsageFlagBits::eVertexBuffer, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
    normalBuffer->fill(normals.data());

    indexBuffer = bufferFactory.createBuffer(indices.size() * sizeof(uint16_t),
                                             vk::BufferUsageFlagBits::eIndexBuffer, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
    indexBuffer->fill(indices.data());

    pipelineFactory.reset();

    pipelineFactory.addVertexInputBinding({0, sizeof(glm::vec3), vk::VertexInputRate::eVertex}); // position
    pipelineFactory.addVertexInputBinding({1, sizeof(glm::vec3), vk::VertexInputRate::eVertex}); // normal
    pipelineFactory.addVertexInputBinding({2, sizeof(glm::vec2), vk::VertexInputRate::eVertex}); // uv
    pipelineFactory.parseShader("tri_vert.spv", "tri_frag.spv", layoutCache, false);
    //// END VERTEX INPUT
    pipelineFactory.getRasterizer().setCullMode(vk::CullModeFlagBits::eBack);
    triPipeline = pipelineFactory.buildPipeline(renderPass.get(), shader2.get());

    vulkanContext.swapChain->createFramebuffers(renderPass->getRenderPass(), depthBuffer->imageView);

    syncer.create(device);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        frames.emplace_back(device, commandPool, syncer);
        auto flags = static_cast<VmaAllocationCreateFlagBits>(VMA_ALLOCATION_CREATE_MAPPED_BIT |
                                                                                    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
        cameraBuffers.emplace_back(bufferFactory.createBuffer(sizeof(CameraMatrices),
                                                              vk::BufferUsageFlagBits::eUniformBuffer,
                                                              flags));
        allocators.emplace_back(vulkanContext.device.logicalDevice);
    }

    model = GLTFLoader::create(commandPool,  vulkanContext.queueManager, bufferFactory, "higokumaru.glb");

    // dummy texture
    uint32_t white = 0xff48ff00;

    textureImage = bufferFactory.createTextureImage(commandPool, vulkanContext.queueManager,
                                                    vk::Format::eR8G8B8A8Srgb, {1, 1, 1},
                                               vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                                                    vk::ImageAspectFlagBits::eColor, &white);
    texture = mkU<Texture>(textureImage.get());
    texture->createSampler();
}

void VulkanApp::recordCommandBuffer(vk::Framebuffer framebuffer)
{
    // TODO: multi thread secondary command buffer recording
    auto &frame = frames[currentFrame];
    auto primaryCommandBuffer = frame.commandBuffer;
    auto swapChainExtent = vulkanContext.swapChain->getExtent();

    updateCameraBuffer();

    //// BEGIN RECORDING COMMAND BUFFER
    frame.begin(); // begins the frames command buffer. for secondary command buffer, need to modify the beginInfo struct

    //// BEGIN RENDER PASS
    renderPass->setFramebuffer(framebuffer);
    renderPass->setRenderAreaExtent(swapChainExtent);
    renderPass->clear(ui->clearColor[0], ui->clearColor[1], ui->clearColor[2], 1.0f);
    renderPass->begin(primaryCommandBuffer);

    auto pipeline = ui->currentShaderIndex == 1 ? pathPipeline.get() : triPipeline.get();
    primaryCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->getGraphicsPipeline());
    auto time = (float)glfwGetTime();
    pathPipeline->setPushConstant(primaryCommandBuffer, &time, sizeof(float), 0);

    ScreenUtils::setViewport(primaryCommandBuffer, swapChainExtent.width, swapChainExtent.height);
    ScreenUtils::setScissor(primaryCommandBuffer, swapChainExtent);

    if (pipeline == triPipeline.get())
    {
        modelTransform = glm::mat4();
        triPipeline->setPushConstant(primaryCommandBuffer, &modelTransform, sizeof(glm::mat4), 0);

        //bind(commandBuffer, {positionBuffer.get(), normalBuffer.get()});
        //indexBuffer->bind(commandBuffer);

        auto bufferInfo = cameraBuffers[currentFrame]->getDescriptorInfo();
        auto textureInfo = texture->getDescriptorInfo();

        auto &allocator = allocators[currentFrame];
        allocator.resetPools();
        vk::DescriptorSet cameraSet;
        vk::DescriptorSetLayout layout;
        // build set by set... all the bindings must be updated / accounted for, but they're all part of the same set anyways
        DescriptorBuilder::beginSet(&layoutCache, &allocator)
                .bindBuffer(0, &bufferInfo, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex)
                .build(cameraSet, layout);

        //std::vector<vk::DescriptorImageInfo> imageInfos = {textureInfo, textureInfo};
        // get image infos of al model textures
        std::vector<vk::DescriptorImageInfo> imageInfos = model->getDescriptorImageInfo();
        vk::DescriptorSet samplerSet;
        DescriptorBuilder::beginSet(&layoutCache, &allocator)
                .bindImage(0, imageInfos,
                           16, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
                .build(samplerSet, layout);

        primaryCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->getPipelineLayout(),
                                                0, {cameraSet, samplerSet}, nullptr);

        //commandBuffer.drawIndexed(6, 1, 0, 0, 0);

//        modelTransform = glm::translate(glm::mat4(), glm::vec3(0.f, 0.f, -1.f));
//        triPipeline->setPushConstant(commandBuffer, &modelTransform, sizeof(glm::mat4));
//        commandBuffer.drawIndexed(6, 1, 0, 0, 0);

        //triPipeline->setPushConstant(commandBuffer, &model->transform, sizeof(glm::mat4));
        //model->draw(commandBuffer);
        model->root->draw(primaryCommandBuffer, *triPipeline, *model->root);
    }
    else
        primaryCommandBuffer.draw(3, 1, 0, 0);

    //// DRAW COMMAND(S)

    ui->end(primaryCommandBuffer);

    //// END RENDER PASS
    renderPass->end(); // ends the render pass with the command buffer given in .begin()

    //// END RECORDING COMMAND BUFFER
    frame.end(); // ends the frames command buffer
}

void VulkanApp::render()
{
    handleInput();
    camera.lerp(ImGui::GetIO().DeltaTime);

    auto &frame = frames[currentFrame];
    frame.finish(syncer); // waits for fence to be signalled (finished frame) and resets the frames inFlightFence

    auto commandBuffer = frame.commandBuffer;

    auto &swapChain = vulkanContext.swapChain;
    // gets next available image, returns the corresponding framebuffer and updates internal index to next image
    auto fb = swapChain->nextImage(frame.imageAvailableSemaphore);

    commandBuffer.reset();
    recordCommandBuffer(fb);

    auto &queueManager = vulkanContext.queueManager;
    // lots of information about syncing in frame.getSubmitInfo() struct.
    queueManager.submitGraphics(frame.getSubmitInfo(), frame.inFlightFence); // signal fence when done

    std::vector<vk::Semaphore> signalSemaphores = { frame.renderFinishedSemaphore };
    // calls the present command, waits for given semaphores
    queueManager.present(swapChain, signalSemaphores);

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanApp::resize()
{
    // recreate swap chain
}

ImGuiCreateParameters VulkanApp::getImGuiCreateParameters()
{
    ImGuiCreateParameters params{};
    params.context = &vulkanContext;
    params.renderPass = renderPass.get();
    params.framesInFlight = MAX_FRAMES_IN_FLIGHT;
    return params;
}

CommandPool& VulkanApp::getCommandPool()
{
    return commandPool;
}

void VulkanApp::updateCameraBuffer()
{
    auto const options = camera.options;
    auto view = camera.getViewMatrix();
    auto extent = vulkanContext.swapChain->getExtent();
    float aspect = extent.width / (float) extent.height;
    auto proj = glm::perspective(glm::radians(options.fov), aspect, options.nearClip, options.farClip);
    proj[1][1] *= -1;
    cameraMatrices.position = glm::vec4(camera.getPosition(), 1.f);
    cameraMatrices.viewProj = proj * view;
    cameraBuffers[currentFrame]->fill(&cameraMatrices);
}

void VulkanApp::handleInput()
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse)
    {
        InputProcesser::setUserPointer(nullptr);
    }
    else
    {
        InputProcesser::setUserPointer(&camera);
        if (InputProcesser::mouseButtons[GLFW_MOUSE_BUTTON_LEFT] ||
             InputProcesser::mouseButtons[GLFW_MOUSE_BUTTON_RIGHT])
        {
            InputProcesser::disableCursor();
        }
        else if (InputProcesser::getCursorState() == GLFW_CURSOR_DISABLED)
        {
            InputProcesser::enableCursor();
        }
    }
}
