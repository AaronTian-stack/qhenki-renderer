#include "vulkanapp.h"

#include "vulkan/screenutils.h"
#include "glm/glm.hpp"
#include "inputprocesser.h"
#include "imgui/imgui.h"
#include <thread>

VulkanApp::VulkanApp() {}

VulkanApp::~VulkanApp()
{
    vulkanContext.device.logicalDevice.waitIdle(); // wait for operations to finish before disposing
    
    for (auto &frame : frames)
        frame.destroy();

    syncer.destroy();
    graphicsCommandPool.destroy();
    transferCommandPool.destroy();

    pathPipeline->destroy();
    pathtraceShader->destroy();

    triPipeline->destroy();
    triShader->destroy();

    gBufferPipeline->destroy();
    gBufferShader->destroy();

    lightingPipeline->destroy();
    lightingShader->destroy();

    pipelineFactory.destroy();

    offscreenRenderPass->destroy();
    displayRenderPass->destroy();
    renderPassBuilder.destroy();

    depthBuffer->destroy();

    for (auto &model : models)
        model->destroy();

    for (auto &cameraBuffer : cameraBuffers)
        cameraBuffer->destroy();

    for (auto &allocator : allocators)
        allocator.destroy();

    layoutCache.destroy();
    bufferFactory.destroy();
}

void VulkanApp::createGbuffer(vk::Extent2D extent, vk::RenderPass renderPass)
{
    // create each attachment

    // position will be reconstructed in the shader

    auto colorAttachment = bufferFactory.createAttachment(vk::Format::eR8G8B8A8Srgb,
                                                          {extent.width, extent.height, 1},
                                                          vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment,
                                                          vk::ImageAspectFlagBits::eColor);
    auto normalAttachment = bufferFactory.createAttachment(vk::Format::eR8G8B8A8Unorm,
                                                           {extent.width, extent.height, 1},
                                                           vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment,
                                                           vk::ImageAspectFlagBits::eColor);
    // metal R roughness G ao B
    auto metalRoughnessAOAttachment = bufferFactory.createAttachment(vk::Format::eR8G8B8A8Unorm,
                                                                    {extent.width, extent.height, 1},
                                                                    vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment,
                                                                    vk::ImageAspectFlagBits::eColor);

    // TODO: emission, specular map

    auto outputAttachment = bufferFactory.createAttachment(vulkanContext.swapChain->getFormat(),
                                                           {extent.width, extent.height, 1},
                                                           vk::ImageUsageFlagBits::eColorAttachment,
                                                           vk::ImageAspectFlagBits::eColor);

    std::vector<vk::ImageView> attachments = {colorAttachment->imageView, // 0
                                              normalAttachment->imageView, // 1
                                              metalRoughnessAOAttachment->imageView, // 2
                                              depthBuffer->imageView, // 3
                                              outputAttachment->imageView // 4
                                            };
    auto device = vulkanContext.device.logicalDevice;
    vk::FramebufferCreateInfo createInfo(
            vk::FramebufferCreateFlags(),
            renderPass,
            attachments.size(),
            attachments.data(),
            extent.width,
            extent.height,
            1);
    auto framebuffer = device.createFramebuffer(createInfo);

    std::vector<sPtr<FrameBufferAttachment>> attachmentsVec = {colorAttachment, normalAttachment, metalRoughnessAOAttachment};
    gBuffer = mkU<FrameBuffer>(device, framebuffer, attachmentsVec);
}

void VulkanApp::create(Window &window)
{
    InputProcesser::setCallbacks(window);

    vulkanContext.create(window);

    layoutCache.create(vulkanContext.device.logicalDevice);
    allocators.emplace_back(vulkanContext.device.logicalDevice);

    auto device = vulkanContext.device.logicalDevice;
    graphicsCommandPool.create(vulkanContext.device, vulkanContext.queueManager.getGraphicsIndex());
    transferCommandPool.create(vulkanContext.device, vulkanContext.queueManager.getTransferIndex());
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
    renderPassBuilder.addSubPass({}, {}, {0}, {}, 1);
    displayRenderPass = renderPassBuilder.buildRenderPass();

    renderPassBuilder.reset();
    renderPassBuilder.addColorAttachment(vk::Format::eR8G8B8A8Srgb); // albedo 0
    renderPassBuilder.addColorAttachment(vk::Format::eR8G8B8A8Unorm); // normal 1
    renderPassBuilder.addColorAttachment(vk::Format::eR8G8B8A8Unorm); // metal roughness ao 2
    renderPassBuilder.addDepthAttachment(depthBuffer->format); // 3
    renderPassBuilder.addColorAttachment(vulkanContext.swapChain->getFormat()); // final output 4
    renderPassBuilder.addSubPass({}, {}, {0, 1, 2}, {}, 3);
    renderPassBuilder.addSubPass({0, 1, 2},
                                 {vk::ImageLayout::eShaderReadOnlyOptimal},
                                 {4}, {}
                                 );
    renderPassBuilder.addColorDependency(0, 1);
    offscreenRenderPass = renderPassBuilder.buildRenderPass();

    pathtraceShader = mkU<Shader>(device, "pathtrace_vert.spv", "pathtrace_frag.spv");
    triShader = mkU<Shader>(device, "tri_vert.spv", "tri_frag.spv");

    pipelineFactory.parseShader("pathtrace_vert.spv", "pathtrace_frag.spv", layoutCache, false);
    pipelineFactory.getColorBlending().attachmentCount = 1;
    pathPipeline = pipelineFactory.buildPipeline(displayRenderPass.get(), 0, pathtraceShader.get());

    gBufferShader = mkU<Shader>(device, "gbuffer_vert.spv", "gbuffer_frag.spv");
    pipelineFactory.reset();
    pipelineFactory.addVertexInputBinding({0, sizeof(glm::vec3), vk::VertexInputRate::eVertex}); // position
    pipelineFactory.addVertexInputBinding({1, sizeof(glm::vec3), vk::VertexInputRate::eVertex}); // normal
    pipelineFactory.addVertexInputBinding({2, sizeof(glm::vec2), vk::VertexInputRate::eVertex}); // uv
    pipelineFactory.parseShader("gbuffer_vert.spv", "gbuffer_frag.spv", layoutCache, false);
    pipelineFactory.getColorBlending().attachmentCount = 3; // blending is disabled for now. pipeline factory does not set color blendings correctly
    gBufferPipeline = pipelineFactory.buildPipeline(offscreenRenderPass.get(), 0, gBufferShader.get());

    lightingShader = mkU<Shader>(device, "lighting_vert.spv", "lighting_frag.spv");
    pipelineFactory.reset();
    pipelineFactory.parseShader("lighting_vert.spv", "lighting_frag.spv", layoutCache, false);
    pipelineFactory.getColorBlending().attachmentCount = 1;
    lightingPipeline = pipelineFactory.buildPipeline(offscreenRenderPass.get(), 1, lightingShader.get());

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

    pipelineFactory.reset();

    pipelineFactory.addVertexInputBinding({0, sizeof(glm::vec3), vk::VertexInputRate::eVertex}); // position
    pipelineFactory.addVertexInputBinding({1, sizeof(glm::vec3), vk::VertexInputRate::eVertex}); // normal
    pipelineFactory.addVertexInputBinding({2, sizeof(glm::vec2), vk::VertexInputRate::eVertex}); // uv
    pipelineFactory.parseShader("tri_vert.spv", "tri_frag.spv", layoutCache, false);
    pipelineFactory.getRasterizer().setCullMode(vk::CullModeFlagBits::eBack);
    pipelineFactory.getColorBlending().attachmentCount = 1;
    triPipeline = pipelineFactory.buildPipeline(displayRenderPass.get(), 0, triShader.get());

    vulkanContext.swapChain->createFramebuffers(displayRenderPass->getRenderPass(), depthBuffer->imageView);

    createGbuffer(vulkanContext.swapChain->getExtent(), offscreenRenderPass->getRenderPass());

    syncer.create(device);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        frames.emplace_back(device, graphicsCommandPool, syncer);
        auto flags = static_cast<VmaAllocationCreateFlagBits>(VMA_ALLOCATION_CREATE_MAPPED_BIT |
                                                                                    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
        cameraBuffers.emplace_back(bufferFactory.createBuffer(sizeof(CameraMatrices),
                                                              vk::BufferUsageFlagBits::eUniformBuffer,
                                                              flags));
        allocators.emplace_back(vulkanContext.device.logicalDevice);
    }
}

void VulkanApp::setUpCallbacks() {
    ui->modelSelectCallback = [this](const std::string& filePath)
    {
        setModel(filePath);
    };
}

void VulkanApp::setModel(const std::string& filePath)
{
    // reset camera position
    camera.simpleReset();
    readyToRender = false;
    std::thread t([this, filePath](){
        models.push_back(GLTFLoader::create(transferCommandPool,  vulkanContext.queueManager,
                                            bufferFactory, filePath.c_str()));
        readyToRender = true;
    });
    t.detach();
}

void VulkanApp::recordOffscreenBuffer(vk::CommandBuffer commandBuffer)
{
    if (readyToRender && !models.empty())
    {
        auto extent = vulkanContext.swapChain->getExtent();
        offscreenRenderPass->setFramebuffer(gBuffer->framebuffer);
        offscreenRenderPass->setRenderAreaExtent(vulkanContext.swapChain->getExtent());
        offscreenRenderPass->clear(ui->clearColor[0], ui->clearColor[1], ui->clearColor[2], 1.0f);
        offscreenRenderPass->begin(commandBuffer);
        ScreenUtils::setViewport(commandBuffer, extent.width, extent.height);
        ScreenUtils::setScissor(commandBuffer, extent);

        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, gBufferPipeline->getGraphicsPipeline());
        auto &model = models.back();
        // get image infos of all model textures
        std::vector<vk::DescriptorImageInfo> imageInfos = model->getDescriptorImageInfo();
        vk::DescriptorSet samplerSet;
        vk::DescriptorSetLayout layout;
        auto &allocator = allocators[currentFrame]; // TODO: maybe change to argument

        auto bufferInfo = cameraBuffers[currentFrame]->getDescriptorInfo();
        vk::DescriptorSet cameraSet;
        DescriptorBuilder::beginSet(&layoutCache, &allocator)
                .bindBuffer(0, &bufferInfo, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex)
                .build(cameraSet, layout);

        DescriptorBuilder::beginSet(&layoutCache, &allocator)
                .bindImage(0, imageInfos,
                           60, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
                .build(samplerSet, layout);

        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, gBufferPipeline->getPipelineLayout(),
                                                0, {cameraSet, samplerSet}, nullptr);

        model->root->draw(commandBuffer, *gBufferPipeline, *model->root);

        offscreenRenderPass->nextSubpass();

        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, lightingPipeline->getGraphicsPipeline());
        // TODO: bind
        vk::DescriptorSet inputSet;
        std::vector<vk::DescriptorImageInfo> inputImageInfos = {
                gBuffer->attachments[0]->getDescriptorInfo(),
                gBuffer->attachments[1]->getDescriptorInfo(),
                gBuffer->attachments[2]->getDescriptorInfo()
        };
        DescriptorBuilder::beginSet(&layoutCache, &allocator)
                .bindImage(0, {inputImageInfos[0]},
                           1, vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment)
                .bindImage(1, {inputImageInfos[1]},
                           1, vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment)
                .bindImage(2, {inputImageInfos[2]},
                           1, vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment)
                .build(inputSet, layout);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, lightingPipeline->getPipelineLayout(),
                                         0, {inputSet}, nullptr);
        commandBuffer.draw(3, 1, 0, 0);

        offscreenRenderPass->end();
    }
}

void VulkanApp::recordCommandBuffer(vk::Framebuffer framebuffer)
{
    // TODO: multi thread secondary command buffer recording
    auto &frame = frames[currentFrame];
    auto primaryCommandBuffer = frame.commandBuffer;
    auto swapChainExtent = vulkanContext.swapChain->getExtent();

    updateCameraBuffer();

    //// BEGIN RECORDING COMMAND BUFFER
    frame.begin(); // begins the frames command buffer

    //// BEGIN RENDER PASS
    displayRenderPass->setFramebuffer(framebuffer);
    displayRenderPass->setRenderAreaExtent(swapChainExtent);
    displayRenderPass->clear(ui->clearColor[0], ui->clearColor[1], ui->clearColor[2], 1.0f);
    displayRenderPass->begin(primaryCommandBuffer);

    auto pipeline = ui->currentShaderIndex == 1 ? pathPipeline.get() : triPipeline.get();
    primaryCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->getGraphicsPipeline());
    auto time = (float)glfwGetTime();
    pathPipeline->setPushConstant(primaryCommandBuffer, &time, sizeof(float), 0, vk::ShaderStageFlagBits::eFragment);

    ScreenUtils::setViewport(primaryCommandBuffer, swapChainExtent.width, swapChainExtent.height);
    ScreenUtils::setScissor(primaryCommandBuffer, swapChainExtent);

    if (pipeline == triPipeline.get())
    {
        auto bufferInfo = cameraBuffers[currentFrame]->getDescriptorInfo();

        auto &allocator = allocators[currentFrame];
        allocator.resetPools();
        vk::DescriptorSet cameraSet;
        vk::DescriptorSetLayout layout;
        // build set by set... all the bindings must be updated / accounted for, but they're all part of the same set anyways
        DescriptorBuilder::beginSet(&layoutCache, &allocator)
                .bindBuffer(0, &bufferInfo, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex)
                .build(cameraSet, layout);

        if (readyToRender && !models.empty())
        {
            auto &model = models.back();
            // get image infos of all model textures
            std::vector<vk::DescriptorImageInfo> imageInfos = model->getDescriptorImageInfo();
            vk::DescriptorSet samplerSet;
            DescriptorBuilder::beginSet(&layoutCache, &allocator)
                    .bindImage(0, imageInfos,
                               60, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
                    .build(samplerSet, layout);

            primaryCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->getPipelineLayout(),
                                                    0, {cameraSet, samplerSet}, nullptr);

            model->root->draw(primaryCommandBuffer, *triPipeline, *model->root);
        }
    }
    else
        primaryCommandBuffer.draw(3, 1, 0, 0);

    //// DRAW COMMAND(S)

    ui->end(primaryCommandBuffer);

    //// END RENDER PASS
    displayRenderPass->end(); // ends the render pass with the command buffer given in .begin()

    recordOffscreenBuffer(primaryCommandBuffer);

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

    // for deferred rendering, we need to
    // 1. wait for swapchain image semaphore
    // 2. render offscreen and signal semaphore (subpass will order itself)
    // 3. wait for offscreen render to finish and then present it

    auto &queueManager = vulkanContext.queueManager;
    // lots of information about syncing in frame.getSubmitInfo() struct.
    queueManager.submitGraphics(frame.getSubmitInfo(), frame.inFlightFence); // signal fence when done

    std::vector<vk::Semaphore> signalSemaphores = {frame.renderFinishedSemaphore};
    // calls the present command, waits for given semaphores
    queueManager.present(swapChain, signalSemaphores);

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    if (models.size() > 1)
    {
        // remove first model
        models[0]->destroy();
        models.erase(models.begin());
    }
}

void VulkanApp::resize()
{
    // recreate swap chain
}

ImGuiCreateParameters VulkanApp::getImGuiCreateParameters()
{
    ImGuiCreateParameters params{};
    params.context = &vulkanContext;
    params.renderPass = displayRenderPass.get();
    params.framesInFlight = MAX_FRAMES_IN_FLIGHT;
    return params;
}

CommandPool& VulkanApp::getGraphicsCommandPool()
{
    return graphicsCommandPool;
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
