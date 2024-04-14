#include "vulkanapp.h"

#include "vulkan/screenutils.h"
#include "glm/glm.hpp"
#include "inputprocesser.h"
#include "imgui/imgui.h"
#include "models/primitives/primitive.h"
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

    gBufferPipeline->destroy();
    gBufferShader->destroy();

    lightingPipeline->destroy();
    lightingShader->destroy();

    postProcessPipeline->destroy();
    postProcessShader->destroy();

    pipelineFactory.destroy();

    offscreenRenderPass->destroy();
    displayRenderPass->destroy();
    renderPassBuilder.destroy();

    gBuffer->destroy();
    depthBuffer->destroy();

    envMap.destroy();

    Primitive::destroy();
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

    auto colorAttachment = bufferFactory.createAttachment(vk::Format::eR8G8B8A8Unorm,
                                                          {extent.width, extent.height, 1},
                                                          vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment,
                                                          vk::ImageAspectFlagBits::eColor);

    auto normalAttachment = bufferFactory.createAttachment(vk::Format::eR8G8B8A8Snorm,
                                                           {extent.width, extent.height, 1},
                                                           vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment,
                                                           vk::ImageAspectFlagBits::eColor);
    // metal R roughness G ao B
    auto metalRoughnessAOAttachment = bufferFactory.createAttachment(vk::Format::eR8G8B8A8Unorm,
                                                                    {extent.width, extent.height, 1},
                                                                    vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment,
                                                                    vk::ImageAspectFlagBits::eColor);

    auto emissiveAttachment = bufferFactory.createAttachment(vk::Format::eR8G8B8A8Unorm,
                                                                     {extent.width, extent.height, 1},
                                                                     vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment,
                                                                     vk::ImageAspectFlagBits::eColor);


    auto outputAttachment = bufferFactory.createAttachment(vk::Format::eR16G16B16A16Sfloat,
                                                           {extent.width, extent.height, 1},
                                                           vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
                                                           vk::ImageAspectFlagBits::eColor);

    outputAttachment->createGenericSampler();

    std::vector<vk::ImageView> attachments = {
            colorAttachment->imageView, // 0
            normalAttachment->imageView, // 1
            metalRoughnessAOAttachment->imageView, // 2
            emissiveAttachment->imageView, // 3
            depthBuffer->imageView, // 4
            outputAttachment->imageView // 5
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

    std::vector<sPtr<FrameBufferAttachment>> attachmentsVec = {colorAttachment, normalAttachment,
                                                               metalRoughnessAOAttachment, emissiveAttachment, outputAttachment};
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
                                                   vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eInputAttachment,
                                                   vk::ImageAspectFlagBits::eDepth);

    renderPassBuilder.create(vulkanContext.device.logicalDevice);

    renderPassBuilder.reset();
    // create render pass
    renderPassBuilder.addColorAttachment(vulkanContext.swapChain->getFormat());
//    renderPassBuilder.addDepthAttachment(depthBuffer->format);
    renderPassBuilder.addSubPass({}, {}, {0}, {});
    displayRenderPass = renderPassBuilder.buildRenderPass();

    renderPassBuilder.reset();
    renderPassBuilder.addColorAttachment(vk::Format::eR8G8B8A8Unorm); // albedo 0
    renderPassBuilder.addColorAttachment(vk::Format::eR8G8B8A8Snorm); // normal 1
    renderPassBuilder.addColorAttachment(vk::Format::eR8G8B8A8Unorm); // metal roughness ao 2
    renderPassBuilder.addColorAttachment(vk::Format::eR8G8B8A8Unorm); // emissive 3
    renderPassBuilder.addDepthAttachment(depthBuffer->format); // depth 4
    renderPassBuilder.addColorAttachment(vk::Format::eR16G16B16A16Sfloat); // final output 5
    renderPassBuilder.addSubPass({}, {}, {0, 1, 2, 3}, {}, 4);
    renderPassBuilder.addSubPass({0, 1, 2, 3, 4},
                                 {vk::ImageLayout::eShaderReadOnlyOptimal},
                                 {5}, {}
                                 );
    renderPassBuilder.addColorDependency(0, 1);
//    renderPassBuilder.addDepthDependency(0, 0);
    offscreenRenderPass = renderPassBuilder.buildRenderPass();

    gBufferShader = mkU<Shader>(device, "gbuffer_vert.spv", "gbuffer_frag.spv");
    pipelineFactory.reset();
    pipelineFactory.addVertexInputBinding({0, sizeof(glm::vec3), vk::VertexInputRate::eVertex}); // position
    pipelineFactory.addVertexInputBinding({1, sizeof(glm::vec3), vk::VertexInputRate::eVertex}); // normal
    pipelineFactory.addVertexInputBinding({2, sizeof(glm::vec3), vk::VertexInputRate::eVertex}); // tangent
    pipelineFactory.addVertexInputBinding({3, sizeof(glm::vec2), vk::VertexInputRate::eVertex}); // uv
    pipelineFactory.parseShader("gbuffer_vert.spv", "gbuffer_frag.spv", layoutCache, false);
    pipelineFactory.getColorBlending().attachmentCount = 4; // blending is disabled for now. pipeline factory does not set color blendings correctly
//    pipelineFactory.getRasterizer().cullMode = vk::CullModeFlagBits::eBack;
    gBufferPipeline = pipelineFactory.buildPipeline(offscreenRenderPass.get(), 0, gBufferShader.get());

    lightingShader = mkU<Shader>(device, "lighting_vert.spv", "lighting_frag.spv");
    pipelineFactory.reset();
    pipelineFactory.parseShader("lighting_vert.spv", "lighting_frag.spv", layoutCache, false);
    pipelineFactory.getDepthStencil().depthWriteEnable = VK_FALSE;
    pipelineFactory.getColorBlending().attachmentCount = 1;
    lightingPipeline = pipelineFactory.buildPipeline(offscreenRenderPass.get(), 1, lightingShader.get());

    postProcessShader = mkU<Shader>(device, "passthrough_vert.spv", "passthrough_frag.spv");
    pipelineFactory.reset();
    pipelineFactory.parseShader("passthrough_vert.spv", "passthrough_frag.spv", layoutCache, false);
    pipelineFactory.getDepthStencil().depthWriteEnable = VK_FALSE;
    pipelineFactory.getColorBlending().attachmentCount = 1;
    postProcessPipeline = pipelineFactory.buildPipeline(displayRenderPass.get(), 0, postProcessShader.get());

    vulkanContext.swapChain->createFramebuffers(displayRenderPass->getRenderPass());

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

    envMap.create(bufferFactory, graphicsCommandPool, vulkanContext.queueManager, "../resources/envmaps/small_room_radiance.dds");

    Primitive::create(bufferFactory);
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
    std::thread t([this, filePath](){
        models.push_back(GLTFLoader::create(transferCommandPool,  vulkanContext.queueManager,
                                            bufferFactory, filePath.c_str()));
        GLTFLoader::setLoadStatus(LoadStatus::READY);
    });
    t.detach();
}

void VulkanApp::recordOffscreenBuffer(vk::CommandBuffer commandBuffer, DescriptorAllocator &allocator)
{
    auto extent = vulkanContext.swapChain->getExtent();
    offscreenRenderPass->setFramebuffer(gBuffer->framebuffer);
    offscreenRenderPass->setRenderAreaExtent(vulkanContext.swapChain->getExtent());
    offscreenRenderPass->clear(ui->clearColor[0], ui->clearColor[1], ui->clearColor[2], 0.0f);
    offscreenRenderPass->begin(commandBuffer);
    ScreenUtils::setViewport(commandBuffer, extent.width, extent.height);
    ScreenUtils::setScissor(commandBuffer, extent);

    if (GLTFLoader::getLoadStatus() == LoadStatus::READY && !models.empty())
    {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, gBufferPipeline->getGraphicsPipeline());
        auto &model = models.back();
        // get image infos of all model textures
        std::vector<vk::DescriptorImageInfo> imageInfos = model->getDescriptorImageInfo();
        vk::DescriptorSet samplerSet;
        vk::DescriptorSetLayout layout;

        auto bufferInfo = cameraBuffers[currentFrame]->getDescriptorInfo();
        vk::DescriptorSet cameraSet;
        DescriptorBuilder::beginSet(&layoutCache, &allocator)
                .bindBuffer(0, &bufferInfo, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex)
                .build(cameraSet, layout);

        if (!imageInfos.empty()) // pipeline still expects uv though
        {
            DescriptorBuilder::beginSet(&layoutCache, &allocator)
                    .bindImage(0, imageInfos,
                               80, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
                    .build(samplerSet, layout);

            commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, gBufferPipeline->getPipelineLayout(),
                                             0, {cameraSet, samplerSet}, nullptr);
        }

        model->root->draw(model->root.get(), commandBuffer, *gBufferPipeline);

        offscreenRenderPass->nextSubpass();

        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, lightingPipeline->getGraphicsPipeline());

        std::vector<std::vector<vk::DescriptorImageInfo>> gBufferInfo =
        {
            {gBuffer->attachments[0]->getDescriptorInfo()},
            {gBuffer->attachments[1]->getDescriptorInfo()},
            {gBuffer->attachments[2]->getDescriptorInfo()},
            {gBuffer->attachments[3]->getDescriptorInfo()},
            {depthBuffer->getDescriptorInfo()}
        };
        vk::DescriptorSet inputSet;
        auto builder = DescriptorBuilder::beginSet(&layoutCache, &allocator);
        for (int i = 0; i <= 4; i++)
        {
            builder.bindImage(i, gBufferInfo[i],
                              1, vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment);
        }
        builder.build(inputSet, layout);

        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, lightingPipeline->getPipelineLayout(),
                                         0, {cameraSet, inputSet}, nullptr);
        commandBuffer.draw(3, 1, 0, 0);
    }
    else offscreenRenderPass->nextSubpass();

    offscreenRenderPass->end();
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

    auto &allocator = allocators[currentFrame];
    allocator.resetPools();
    recordOffscreenBuffer(primaryCommandBuffer, allocator);

    auto &outputAttachment = gBuffer->attachments.back();
    Image::recordTransitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal,
                                       outputAttachment->image,primaryCommandBuffer, 1, 1);

    displayRenderPass->setFramebuffer(framebuffer);
    displayRenderPass->setRenderAreaExtent(swapChainExtent);
    displayRenderPass->clear(ui->clearColor[0], ui->clearColor[1], ui->clearColor[2], 1.0f);
    displayRenderPass->begin(primaryCommandBuffer);

    primaryCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, postProcessPipeline->getGraphicsPipeline());
    ScreenUtils::setViewport(primaryCommandBuffer, swapChainExtent.width, swapChainExtent.height);
    ScreenUtils::setScissor(primaryCommandBuffer, swapChainExtent);
    vk::DescriptorSet samplerSet;
    vk::DescriptorSetLayout layout;
    DescriptorBuilder::beginSet(&layoutCache, &allocator)
            .bindImage(0, {outputAttachment->getDescriptorInfo()}, // output attachment sampler
                       1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
            .build(samplerSet, layout);
    primaryCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, postProcessPipeline->getPipelineLayout(),
                                            0, {samplerSet}, nullptr);
    // get window resolution
    auto extent = vulkanContext.swapChain->getExtent();
    auto resolution = glm::vec2(extent.width, extent.height);
    postProcessPipeline->setPushConstant(primaryCommandBuffer, &resolution, sizeof(glm::vec2), 0, vk::ShaderStageFlagBits::eFragment);
    primaryCommandBuffer.draw(3, 1, 0, 0);

    ui->end(primaryCommandBuffer);
    displayRenderPass->end();

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
    // TODO: recreate swap chain
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
    cameraMatrices.forward = camera.getForwardVector();
    cameraMatrices.viewProj = proj * view;
    cameraMatrices.inverseViewProj = glm::inverse(cameraMatrices.viewProj);
    cameraMatrices.view = view;
    cameraMatrices.proj = proj;
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
