#include "vulkanapp.h"

#include "vulkan/screenutils.h"
#include "glm/glm.hpp"
#include "inputprocesser.h"
#include "imgui/imgui.h"
#include "models/primitives/primitivedrawer.h"
#include "vfx/effects/fxaa.h"
#include "vfx/effects/vignette.h"
#include "vfx/effects/sharpen.h"
#include <thread>

VulkanApp::VulkanApp() {}

VulkanApp::~VulkanApp()
{
    vulkanContext.device.logicalDevice.waitIdle(); // wait for operations to finish before disposing
    
    for (auto &frame : frames)
        frame.destroy();

    syncer.destroy();
    graphicsCommandPool->destroy();
    transferCommandPool->destroy();

    gBufferPipeline->destroy();
    gBufferShader->destroy();

    lightingPipeline->destroy();
    lightingShader->destroy();

    cubeMapPipeline->destroy();
    cubeMapShader->destroy();

    passAndClearPipeline->destroy();
    passAndClearShader->destroy();

    passPipeline->destroy();
    passShader->destroy();

    pipelineFactory.destroy();

    offscreenRenderPass->destroy();
    displayRenderPass->destroy();
    renderPassBuilder.destroy();

    postProcessManager->destroy();

    gBuffer->destroy();
    depthBuffer->destroy();

    envMap.destroy();

    PrimitiveDrawer::destroy();
    for (auto &model : models)
        model->destroy();

    for (auto &cameraBuffer : cameraBuffers)
        cameraBuffer->destroy();

    for (auto &sphereLightBuffer : sphereLightBuffers)
        sphereLightBuffer->destroy();

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

    auto normalAttachment = bufferFactory.createAttachment(vk::Format::eR16G16B16A16Sfloat,
                                                           {extent.width, extent.height, 1},
                                                           vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment,
                                                           vk::ImageAspectFlagBits::eColor);
    // metal R roughness G ao B
    auto metalRoughnessAOAttachment = bufferFactory.createAttachment(vk::Format::eR8G8B8A8Unorm,
                                                                    {extent.width, extent.height, 1},
                                                                    vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment,
                                                                    vk::ImageAspectFlagBits::eColor);

    auto emissiveAttachment = bufferFactory.createAttachment(vk::Format::eR16G16B16A16Sfloat,
                                                                     {extent.width, extent.height, 1},
                                                                     vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment,
                                                                     vk::ImageAspectFlagBits::eColor);


    auto outputAttachment = bufferFactory.createAttachment(vk::Format::eR16G16B16A16Sfloat,
                                                           {extent.width, extent.height, 1},
                                                           vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
                                                           vk::ImageAspectFlagBits::eColor);

    outputAttachment->createGenericSampler();

    gBuffer = mkU<GBuffer>(vulkanContext.device.logicalDevice);

    gBuffer->setAttachment(GBufferAttachmentType::ALBEDO, colorAttachment, true);
    gBuffer->setAttachment(GBufferAttachmentType::NORMAL, normalAttachment, true);
    gBuffer->setAttachment(GBufferAttachmentType::METAL_ROUGHNESS_AO, metalRoughnessAOAttachment, true);
    gBuffer->setAttachment(GBufferAttachmentType::EMISSIVE, emissiveAttachment, true);
    gBuffer->setAttachment(GBufferAttachmentType::DEPTH, depthBuffer, false);
    gBuffer->setAttachment(GBufferAttachmentType::OUTPUT, outputAttachment, true); // this must be last for some reason

    gBuffer->createFrameBuffer(renderPass, extent);
}

void VulkanApp::createRenderPasses()
{
    renderPassBuilder.create(vulkanContext.device.logicalDevice);

    renderPassBuilder.reset();
    renderPassBuilder.addColorAttachment(vulkanContext.swapChain->getFormat());
    renderPassBuilder.addSubPass({}, {}, {0}, {});
    displayRenderPass = renderPassBuilder.buildRenderPass();

    renderPassBuilder.reset();
    renderPassBuilder.addColorAttachment(vk::Format::eR8G8B8A8Unorm); // albedo 0
    renderPassBuilder.addColorAttachment(vk::Format::eR16G16B16A16Sfloat); // normal 1
    renderPassBuilder.addColorAttachment(vk::Format::eR8G8B8A8Unorm); // metal roughness ao 2
    renderPassBuilder.addColorAttachment(vk::Format::eR16G16B16A16Sfloat); // emissive 3
    renderPassBuilder.addDepthAttachment(depthBuffer->format); // depth 4
    renderPassBuilder.addColorAttachment(vk::Format::eR16G16B16A16Sfloat); // final output 5
    renderPassBuilder.addSubPass({}, {}, {0, 1, 2, 3}, {}, 4);
    renderPassBuilder.addSubPass({0, 1, 2, 3, 4},
                                 {vk::ImageLayout::eShaderReadOnlyOptimal},
                                 {5}, {});
    renderPassBuilder.addSubPass({},
                                 {vk::ImageLayout::eShaderReadOnlyOptimal},
                                 {5}, {}, 4);
    renderPassBuilder.addColorDependency(0, 1);
    renderPassBuilder.addColorDependency(1, 2);
    offscreenRenderPass = renderPassBuilder.buildRenderPass();
}

void VulkanApp::createPipelines()
{
    auto device = vulkanContext.device.logicalDevice;

    gBufferShader = mkU<Shader>(device, "gbuffer_vert.spv", "gbuffer_frag.spv");
    pipelineFactory.reset();
    pipelineFactory.addVertexInputBinding({0, sizeof(glm::vec3), vk::VertexInputRate::eVertex}); // position
    pipelineFactory.addVertexInputBinding({1, sizeof(glm::vec3), vk::VertexInputRate::eVertex}); // normal
    pipelineFactory.addVertexInputBinding({2, sizeof(glm::vec3), vk::VertexInputRate::eVertex}); // tangent
    pipelineFactory.addVertexInputBinding({3, sizeof(glm::vec2), vk::VertexInputRate::eVertex}); // uv_0
    pipelineFactory.addVertexInputBinding({4, sizeof(glm::vec2), vk::VertexInputRate::eVertex}); // uv_1
    pipelineFactory.parseShader("gbuffer_vert.spv", "gbuffer_frag.spv", layoutCache, false);
    pipelineFactory.getColorBlending().attachmentCount = 4; // blending is disabled for now. pipeline factory does not set color blendings correctly

    // TODO: some models don't render correctly without culling. investigate
    pipelineFactory.getRasterizer().cullMode = vk::CullModeFlagBits::eBack;
    gBufferPipeline = pipelineFactory.buildPipeline(offscreenRenderPass.get(), 0, gBufferShader.get());

    lightingShader = mkU<Shader>(device, "lighting_vert.spv", "lighting_frag.spv");
    pipelineFactory.reset();
    pipelineFactory.parseShader("lighting_vert.spv", "lighting_frag.spv", layoutCache, false);
    pipelineFactory.getDepthStencil().depthWriteEnable = VK_FALSE;
    pipelineFactory.getColorBlending().attachmentCount = 1;
    lightingPipeline = pipelineFactory.buildPipeline(offscreenRenderPass.get(), 1, lightingShader.get());

    cubeMapShader = mkU<Shader>(device, "cubemap_vert.spv", "cubemap_frag.spv");
    pipelineFactory.reset();
    pipelineFactory.addVertexInputBinding({0, sizeof(glm::vec3), vk::VertexInputRate::eVertex}); // position
    pipelineFactory.parseShader("cubemap_vert.spv", "cubemap_frag.spv", layoutCache, false);
    pipelineFactory.getDepthStencil().depthWriteEnable = VK_FALSE;
    pipelineFactory.getDepthStencil().depthCompareOp = vk::CompareOp::eLessOrEqual; // NEEDS TO BE EXPLICITLY SET
    pipelineFactory.getColorBlending().attachmentCount = 1;
    cubeMapPipeline = pipelineFactory.buildPipeline(offscreenRenderPass.get(), 2, cubeMapShader.get());

    PrimitiveDrawer::create(bufferFactory, vulkanContext.device.logicalDevice, pipelineFactory, offscreenRenderPass.get(), 2);

    passShader = mkU<Shader>(device, "passthrough_vert.spv", "passthrough_frag.spv");
    pipelineFactory.reset();
    pipelineFactory.parseShader("passthrough_vert.spv", "passthrough_frag.spv", layoutCache, false);
    pipelineFactory.getDepthStencil().depthWriteEnable = VK_FALSE;
    pipelineFactory.getColorBlending().attachmentCount = 1;
    passPipeline = pipelineFactory.buildPipeline(displayRenderPass.get(), 0, passShader.get());
}

void VulkanApp::createPostProcess()
{
    auto device = vulkanContext.device.logicalDevice;
    auto extent = vulkanContext.swapChain->getExtent();

    postProcessManager = mkU<PostProcessManager>(vulkanContext.device.logicalDevice, extent, bufferFactory, renderPassBuilder);
    passAndClearShader = mkU<Shader>(device, "passthrough_vert.spv", "passthrough_depth_frag.spv");
    pipelineFactory.reset();
    pipelineFactory.parseShader("passthrough_vert.spv", "passthrough_depth_frag.spv", layoutCache, false);
    pipelineFactory.getDepthStencil().depthWriteEnable = VK_FALSE;
    pipelineFactory.getColorBlending().attachmentCount = 1;
    passAndClearPipeline = pipelineFactory.buildPipeline(&postProcessManager->getPingPongRenderPass(), 0, passAndClearShader.get());
    auto fxaa = mkS<FXAA>(vulkanContext.device.logicalDevice, "fxaa_frag.spv",
                          pipelineFactory, layoutCache, &postProcessManager->getPingPongRenderPass());
    auto vignette = mkS<Vignette>(vulkanContext.device.logicalDevice, "vignette_frag.spv",
                                  pipelineFactory, layoutCache, &postProcessManager->getPingPongRenderPass());
    auto sharpen = mkS<Sharpen>(vulkanContext.device.logicalDevice, "sharpen_frag.spv",
                                pipelineFactory, layoutCache, &postProcessManager->getPingPongRenderPass());
    postProcessManager->addPostProcess(fxaa);
    postProcessManager->addPostProcess(vignette);
    postProcessManager->addPostProcess(sharpen);
    postProcessManager->activatePostProcess(0);
    auto reinhard = mkU<PostProcess>("Reinhard", vulkanContext.device.logicalDevice, "reinhard_frag.spv",
                                     pipelineFactory, layoutCache, &postProcessManager->getPingPongRenderPass());
    auto neutral = mkU<PostProcess>("Khronos PBR Neutral", vulkanContext.device.logicalDevice, "pbr_neutral_frag.spv",
                                    pipelineFactory, layoutCache, &postProcessManager->getPingPongRenderPass());
    auto aces = mkU<PostProcess>("ACES", vulkanContext.device.logicalDevice, "aces_frag.spv",
                                 pipelineFactory, layoutCache, &postProcessManager->getPingPongRenderPass());
    postProcessManager->addToneMapper(reinhard);
    postProcessManager->addToneMapper(neutral);
    postProcessManager->addToneMapper(aces);
}

void VulkanApp::create(Window &window)
{
    InputProcesser::setCallbacks(window);

    vulkanContext.create(window);

    layoutCache.create(vulkanContext.device.logicalDevice);
    allocators.emplace_back(vulkanContext.device.logicalDevice);

    auto device = vulkanContext.device.logicalDevice;
    graphicsCommandPool = mkU<CommandPool>(vulkanContext.device, vkb::QueueType::graphics, vulkanContext.queueManager.getGraphicsIndex());
    transferCommandPool = mkU<CommandPool>(vulkanContext.device, vkb::QueueType::transfer, vulkanContext.queueManager.getTransferIndex());
    pipelineFactory.create(device);

    bufferFactory.create(vulkanContext);
    auto extent = vulkanContext.swapChain->getExtent();
    depthBuffer = bufferFactory.createAttachment(vk::Format::eD32Sfloat, {extent.width, extent.height, 1},
                                                   vk::ImageUsageFlagBits::eDepthStencilAttachment
                                                   | vk::ImageUsageFlagBits::eInputAttachment | vk::ImageUsageFlagBits::eSampled,
                                                   vk::ImageAspectFlagBits::eDepth);
    depthBuffer->createGenericSampler(vk::Filter::eNearest, vk::SamplerMipmapMode::eNearest);

    createRenderPasses();

    createPipelines();

    vulkanContext.swapChain->createFramebuffers(displayRenderPass->getRenderPass());

    createGbuffer(vulkanContext.swapChain->getExtent(), offscreenRenderPass->getRenderPass());

    syncer.create(device);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        frames.emplace_back(device, *graphicsCommandPool, syncer);
        auto flags = static_cast<VmaAllocationCreateFlagBits>(VMA_ALLOCATION_CREATE_MAPPED_BIT |
                                                                                    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
        cameraBuffers.emplace_back(bufferFactory.createBuffer(sizeof(CameraMatrices),
                                                              vk::BufferUsageFlagBits::eUniformBuffer,
                                                              flags));

        sphereLightBuffers.emplace_back(bufferFactory.createBuffer(sizeof(SphereLight) * MAX_LIGHTS,
                                                              vk::BufferUsageFlagBits::eStorageBuffer,
                                                              flags));

        allocators.emplace_back(vulkanContext.device.logicalDevice);
    }

    envMap.create(bufferFactory, *graphicsCommandPool, vulkanContext.queueManager,
                  "../resources/envmaps/artist_workshop/artist_workshop.dds"
//                  "../resources/envmaps/metro_noord/metro_noord.dds"
                  );

    createPostProcess();

    for (int i = 0; i < 1; i++)
    {
        sphereLights.push_back({{1.f, 1.f, 1.f}, {1.f, 1.f, 1.f}, 1.f});
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
    std::thread t([this, filePath](){
        auto newModel = GLTFLoader::create(*transferCommandPool, vulkanContext.queueManager,
                                           bufferFactory, filePath.c_str());
        std::lock_guard lock(modelMutex);
        models.push_back(std::move(newModel));
        GLTFLoader::setLoadPercent(1.f);
    });
    t.detach();
}

void VulkanApp::recordOffscreenBuffer(vk::CommandBuffer commandBuffer, DescriptorAllocator &allocator)
{
    offscreenRenderPass->setFramebuffer(gBuffer->framebuffer);
    offscreenRenderPass->setRenderAreaExtent(vulkanContext.swapChain->getExtent());
    offscreenRenderPass->clear(0.f, 0.f, 0.f, 0.f);
    offscreenRenderPass->begin(commandBuffer);

    vk::DescriptorSetLayout layout;
    vk::DescriptorSet cameraSet;
    auto bufferInfo = cameraBuffers[currentFrame]->getDescriptorInfo();

    DescriptorBuilder::beginSet(&layoutCache, &allocator)
            .bindBuffer(0, &bufferInfo, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex)
            .build(cameraSet, layout);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, gBufferPipeline->getPipelineLayout(),
                                      0, {cameraSet}, nullptr);

    std::unique_lock lock(modelMutex);
    if (!models.empty())
    {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, gBufferPipeline->getGraphicsPipeline());
        auto &model = models.back();
        // get image infos of all model textures
        std::vector<vk::DescriptorImageInfo> imageInfos = model->getDescriptorImageInfo();
        vk::DescriptorSet samplerSet;

        if (!imageInfos.empty()) // pipeline still expects uv though
        {
            const int textureCount = 80;
            if (imageInfos.size() > textureCount) throw std::runtime_error("Too many textures");
            DescriptorBuilder::beginSet(&layoutCache, &allocator)
                    .bindImage(0, imageInfos,
                               textureCount, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
                    .build(samplerSet, layout);

            commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, gBufferPipeline->getPipelineLayout(),
                                             1, {samplerSet}, nullptr);
        }

        Node::draw(model->root, commandBuffer, *gBufferPipeline);
    }
    lock.unlock();

    offscreenRenderPass->nextSubpass(); // composition pass

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, lightingPipeline->getGraphicsPipeline());
    lightingParameters.iblIntensity = ui->iblIntensity;
    lightingParameters.emissionMultiplier = ui->emissionMultiplier;
    lightingPipeline->setPushConstant(commandBuffer, &lightingParameters, sizeof(LightingParameters), 0, vk::ShaderStageFlagBits::eFragment);

    std::vector<std::vector<vk::DescriptorImageInfo>> gBufferInfo =
    {
        {gBuffer->getAttachment(GBufferAttachmentType::ALBEDO)->getDescriptorInfo()},
        {gBuffer->getAttachment(GBufferAttachmentType::NORMAL)->getDescriptorInfo()},
        {gBuffer->getAttachment(GBufferAttachmentType::METAL_ROUGHNESS_AO)->getDescriptorInfo()},
        {gBuffer->getAttachment(GBufferAttachmentType::EMISSIVE)->getDescriptorInfo()},
        {depthBuffer->getDescriptorInfo()}
    };
    vk::DescriptorSet inputSet;
    auto builder = DescriptorBuilder::beginSet(&layoutCache, &allocator);
    for (int i = 0; i <= 4; i++)
    {
        // vectors need to remain in scope until build is called
        builder.bindImage(i, gBufferInfo[i],
                          1, vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment);
    }
    builder.build(inputSet, layout);

    vk::DescriptorSet iblSamplerSet;
    std::vector<std::vector<vk::DescriptorImageInfo>> cubeMapInfos = {
            {envMap.cubeMap.texture->getDescriptorInfo()},
            {envMap.irradianceMap.texture->getDescriptorInfo()},
            {envMap.radianceMap.texture->getDescriptorInfo()},
            {envMap.brdfLUT.texture->getDescriptorInfo()}
    };
    auto cubeBuilder = DescriptorBuilder::beginSet(&layoutCache, &allocator);
    for (int i = 0; i <= 3; i++)
    {
        cubeBuilder.bindImage(i, cubeMapInfos[i],
                              1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
    }
    cubeBuilder.build(iblSamplerSet, layout);

    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, lightingPipeline->getPipelineLayout(),
                                     0, {cameraSet, inputSet, iblSamplerSet}, nullptr);

    commandBuffer.draw(3, 1, 0, 0);

    offscreenRenderPass->nextSubpass(); // cube map pass

    if (ui->drawBackground)
    {
        vk::DescriptorSet cubeSamplerSet;
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, cubeMapPipeline->getGraphicsPipeline());
        DescriptorBuilder::beginSet(&layoutCache, &allocator)
                .bindImage(0, {envMap.cubeMap.texture->getDescriptorInfo()}, // env map sampler
                           1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
                .build(cubeSamplerSet, layout);

        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, cubeMapPipeline->getPipelineLayout(),
                                         0, {cameraSet, cubeSamplerSet}, nullptr);

        PrimitiveDrawer::drawCube(commandBuffer);
    }

    offscreenRenderPass->end();
}

void VulkanApp::recordCommandBuffer(FrameBuffer *framebuffer)
{
    // TODO: multi thread secondary command buffer recording
    auto &frame = frames[currentFrame];
    auto primaryCommandBuffer = frame.commandBuffer;
    auto swapChainExtent = vulkanContext.swapChain->getExtent();

    updateCameraBuffer();
    updateLightBuffers();

    //// BEGIN RECORDING COMMAND BUFFER
    frame.begin(); // begins the frames command buffer

    auto &allocator = allocators[currentFrame];
    allocator.resetPools();
    ScreenUtils::setViewport(primaryCommandBuffer, swapChainExtent.width, swapChainExtent.height);
    ScreenUtils::setScissor(primaryCommandBuffer, swapChainExtent);
    recordOffscreenBuffer(primaryCommandBuffer, allocator);

    auto &outputAttachment = gBuffer->attachments.back();
    Image::recordTransitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal,
                                       outputAttachment->image, primaryCommandBuffer, 1, 1);

    auto outputAttachmentInfo = outputAttachment->getDescriptorInfo();
    postProcessManager->tonemap(primaryCommandBuffer, layoutCache, allocator, &outputAttachmentInfo);
    // transition depth buffer for clear color
    Image::recordTransitionImageLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
                                       depthBuffer->image, primaryCommandBuffer, 1, 1);
    Image::recordTransitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal,
                                       postProcessManager->getAttachment(0)->image, primaryCommandBuffer, 1, 1);

    //// CLEAR COLOR
    auto &ppr = postProcessManager->getPingPongRenderPass();
    ppr.setFramebuffer(postProcessManager->getFramebuffer(1));
    ppr.setRenderAreaExtent(swapChainExtent);
    ppr.clear(0.f, 0.f, 0.f, 1.f);
    ppr.begin(primaryCommandBuffer);
    primaryCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, passAndClearPipeline->getGraphicsPipeline());
    vk::DescriptorSet pprSamplerSet; vk::DescriptorSetLayout layout;
    DescriptorBuilder::beginSet(&layoutCache, &allocator)
            .bindImage(0, {postProcessManager->getAttachment(0)->getDescriptorInfo()},
                       1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
            .bindImage(1, {depthBuffer->getDescriptorInfo()}, // depth attachment sampler
                       1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
            .build(pprSamplerSet, layout);
    primaryCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, passAndClearPipeline->getPipelineLayout(),
                                            0, {pprSamplerSet}, nullptr);
    glm::vec4 clear = {ui->clearColor[0], ui->clearColor[1], ui->clearColor[2], ui->drawBackground ? 0.f : 1.f};
    passAndClearPipeline->setPushConstant(primaryCommandBuffer, &clear, sizeof(glm::vec4), 0, vk::ShaderStageFlagBits::eFragment);
    primaryCommandBuffer.draw(3, 1, 0, 0);
    ppr.end();
    //// END CLEAR COLOR

    postProcessManager->render(primaryCommandBuffer, layoutCache, allocator);

    Image::recordTransitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal,
                                       postProcessManager->getCurrentAttachment()->image, primaryCommandBuffer, 1, 1);

    displayRenderPass->setFramebuffer(framebuffer->framebuffer);
    displayRenderPass->setRenderAreaExtent(swapChainExtent);
    displayRenderPass->clear(0.f, 0.f, 0.f, 1.0f);
    displayRenderPass->begin(primaryCommandBuffer);

    primaryCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, passPipeline->getGraphicsPipeline());
    vk::DescriptorSet samplerSet;
    DescriptorBuilder::beginSet(&layoutCache, &allocator)
            .bindImage(0, {postProcessManager->getCurrentAttachment()->getDescriptorInfo()}, // output attachment sampler
                       1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
            .build(samplerSet, layout);
    primaryCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, passPipeline->getPipelineLayout(),
                                            0, {samplerSet}, nullptr);
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

    attemptToDeleteOldModel();

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
    return *graphicsCommandPool;
}

void VulkanApp::updateCameraBuffer()
{
    auto const options = camera.options;
    auto view = camera.getViewMatrix();
    auto extent = vulkanContext.swapChain->getExtent();
    float aspect = extent.width / (float) extent.height;
    auto proj = glm::perspective(glm::radians(camera.getFOV()), aspect, options.nearClip, options.farClip);
    proj[1][1] *= -1;
    cameraMatrices.position = camera.getPosition();
    cameraMatrices.forward = camera.getForwardVector();
    cameraMatrices.viewProj = proj * view;
    cameraMatrices.inverseViewProj = glm::inverse(cameraMatrices.viewProj);
    cameraMatrices.view = view;
    cameraMatrices.proj = proj;
    auto cb = static_cast<CameraMatrices*>(cameraBuffers[currentFrame]->getPointer());
    *cb = cameraMatrices;
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

void VulkanApp::updateLightBuffers()
{
    auto lightBuffer = static_cast<SphereLight*>(sphereLightBuffers[currentFrame]->getPointer());
    for (int i = 0; i < sphereLights.size(); i++)
    {
        lightBuffer[i] = sphereLights[i];
    }
}

void VulkanApp::attemptToDeleteOldModel()
{
    std::unique_lock lock(modelMutex);
    if (models.size() > 1)
    {
        bool canDelete = true;
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            auto result = vulkanContext.device.logicalDevice.getFenceStatus(frames[0].inFlightFence);
            if (result != vk::Result::eSuccess)
            {
                canDelete = false;
                break;
            }
        }
        if (canDelete)
        {
            // remove first model. it will get deleted. eventually...
            while(models.size() > 1)
            {
                models[0]->destroy();
                models.erase(models.begin());
            }
            std::cout << "DELETED MODEL" << std::endl;
        }
    }
    lock.unlock();
}
