#include "vulkanapp.h"

#include "vulkan/screenutils.h"
#include <glm/glm.hpp>
#include "inputprocesser.h"
#include <imgui/imgui.h>
#include "models/primitives/primitivedrawer.h"
#include "vfx/effects/fxaa.h"
#include "vfx/effects/vignette.h"
#include "vfx/effects/sharpen.h"
#include "vfx/effects/filmgrain.h"
#include "vfx/effects/chromaticaberration.h"
#include "imgui_internal.h"
#include <thread>

VulkanApp::VulkanApp() : drawBackground(true), drawGrid(false), gridScale(35.f), clearColor(0.25f) {}

VulkanApp::~VulkanApp()
{
    vulkanContext.device.logicalDevice.waitIdle(); // wait for operations to finish before disposing
    
    for (auto &frame : frames)
        frame.destroy();

    for (auto &semaphore : computeSemaphores)
        vulkanContext.device.logicalDevice.destroySemaphore(semaphore);

    for (auto &fence : computeFences)
        vulkanContext.device.logicalDevice.destroyFence(fence);

    syncer.destroy();
    graphicsCommandPool->destroy();
    transferCommandPool->destroy();

    gBufferPS.destroy();
    lighting.destroy();
    pass.destroy();
    passAndClear.destroy();
    cubeMap.destroy();
    lightDisplay.destroy();
    solidPlane.destroy();
    gridPlane.destroy();
    
    skinning.destroy();

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

    bayerMatrix->destroy();

    for (auto &cameraBuffer : cameraBuffers)
        cameraBuffer->destroy();

    for (auto &sphereLightBuffer : sphereLightBuffers)
        sphereLightBuffer->destroy();

    for (auto &tubeLightBuffer : tubeLightsBuffers)
        tubeLightBuffer->destroy();

    for (auto &rectangleLightBuffer : rectangleLightBuffers)
        rectangleLightBuffer->destroy();

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

    gBuffer = mkU<GBuffer>(vulkanContext.device.logicalDevice, extent);

    gBuffer->setAttachment(GBufferAttachmentType::ALBEDO, colorAttachment, true);
    gBuffer->setAttachment(GBufferAttachmentType::NORMAL, normalAttachment, true);
    gBuffer->setAttachment(GBufferAttachmentType::METAL_ROUGHNESS_AO, metalRoughnessAOAttachment, true);
    gBuffer->setAttachment(GBufferAttachmentType::EMISSIVE, emissiveAttachment, true);
    gBuffer->setAttachment(GBufferAttachmentType::DEPTH, depthBuffer, false);
    gBuffer->setAttachment(GBufferAttachmentType::OUTPUT, outputAttachment, true); // this must be last for some reason

    gBuffer->createFrameBuffer(renderPass);
}

void VulkanApp::createRenderPasses()
{
    renderPassBuilder.create(vulkanContext.device.logicalDevice);

    // create render pass
    renderPassBuilder.reset();
    renderPassBuilder.addColorAttachment(vulkanContext.swapChain->getFormat());
    renderPassBuilder.addSubPass({}, {}, {0}, {});
    displayRenderPass = renderPassBuilder.buildRenderPass();

    renderPassBuilder.reset();
    renderPassBuilder.addColorAttachment(vk::Format::eR8G8B8A8Unorm); // albedo 0
    renderPassBuilder.addColorAttachment(vk::Format::eR16G16B16A16Sfloat); // normal 1
    renderPassBuilder.addColorAttachment(vk::Format::eR8G8B8A8Unorm); // metal roughness ao 2
    renderPassBuilder.addColorAttachment(vk::Format::eR16G16B16A16Sfloat); // emissive 3

    renderPassBuilder.addDepthAttachment(depthBuffer->format,
                                         vk::AttachmentLoadOp::eClear,
                                         vk::ImageLayout::eShaderReadOnlyOptimal); // depth 4

    renderPassBuilder.addColorAttachment(vk::Format::eR16G16B16A16Sfloat,
                                         vk::AttachmentLoadOp::eClear,
                                         vk::ImageLayout::eShaderReadOnlyOptimal); // final output 5

    renderPassBuilder.addSubPass({}, {}, {0, 1, 2, 3}, {}, 4); // draw to gbuffer

    renderPassBuilder.addSubPass({0, 1, 2, 3, 4},
                                 {vk::ImageLayout::eShaderReadOnlyOptimal},
                                 {5}, {}); // composite

    renderPassBuilder.addSubPass({},
                                 {},
                                 {5}, {}, 4); // draw cube map

    renderPassBuilder.addSubPass({}, {}, {5}, {}, 4); // draw lights

    renderPassBuilder.addColorDependency(0, 1);
    renderPassBuilder.addColorDependency(1, 2);
    renderPassBuilder.addColorDependency(2, 3);
    offscreenRenderPass = renderPassBuilder.buildRenderPass();
}

void VulkanApp::createPipelines()
{
    auto device = vulkanContext.device.logicalDevice;

    gBufferPS.shader = mkU<Shader>(device, "gbuffer.vert", "gbuffer.frag");
    pipelineFactory.reset();
    pipelineFactory.addVertexInputBinding({0, sizeof(glm::vec3), vk::VertexInputRate::eVertex}); // position
    pipelineFactory.addVertexInputBinding({1, sizeof(glm::vec3), vk::VertexInputRate::eVertex}); // normal
    pipelineFactory.addVertexInputBinding({2, sizeof(glm::vec3), vk::VertexInputRate::eVertex}); // tangent
    pipelineFactory.addVertexInputBinding({3, sizeof(glm::vec2), vk::VertexInputRate::eVertex}); // uv_0
    pipelineFactory.addVertexInputBinding({4, sizeof(glm::vec2), vk::VertexInputRate::eVertex}); // uv_1
    pipelineFactory.parseShader("gbuffer.vert", "gbuffer.frag", layoutCache, false);
    pipelineFactory.getColorBlending().attachmentCount = 4; // blending is disabled for now. pipeline factory does not set color blendings correctly

    // TODO: some models don't render correctly without culling. investigate
    pipelineFactory.getRasterizer().cullMode = vk::CullModeFlagBits::eBack;
    gBufferPS.pipeline = pipelineFactory.buildPipeline(offscreenRenderPass.get(), 0, gBufferPS.shader.get());

    lighting.shader = mkU<Shader>(device, "lighting.vert", "lighting.frag");
    pipelineFactory.reset();
    pipelineFactory.parseShader("lighting.vert", "lighting.frag", layoutCache, false);
    pipelineFactory.getDepthStencil().depthWriteEnable = VK_FALSE;
    pipelineFactory.getColorBlending().attachmentCount = 1;
    lighting.pipeline = pipelineFactory.buildPipeline(offscreenRenderPass.get(), 1, lighting.shader.get());

    cubeMap.shader = mkU<Shader>(device, "cubemap.vert", "cubemap.frag");
    pipelineFactory.reset();
    pipelineFactory.addVertexInputBinding({0, sizeof(glm::vec3), vk::VertexInputRate::eVertex}); // position
    pipelineFactory.parseShader("cubemap.vert", "cubemap.frag", layoutCache, false);
    pipelineFactory.getDepthStencil().depthWriteEnable = VK_FALSE;
    pipelineFactory.getDepthStencil().depthCompareOp = vk::CompareOp::eLessOrEqual; // NEEDS TO BE EXPLICITLY SET
    pipelineFactory.getColorBlending().attachmentCount = 1;
    cubeMap.pipeline = pipelineFactory.buildPipeline(offscreenRenderPass.get(), 2, cubeMap.shader.get());

    PrimitiveDrawer::create(bufferFactory);

    pass.shader = mkU<Shader>(device, "passthrough.vert", "passthrough.frag");
    pipelineFactory.reset();
    pipelineFactory.parseShader("passthrough.vert", "passthrough.frag", layoutCache, false);
    pipelineFactory.getDepthStencil().depthWriteEnable = VK_FALSE;
    pipelineFactory.getColorBlending().attachmentCount = 1;
    pass.pipeline = pipelineFactory.buildPipeline(displayRenderPass.get(), 0, pass.shader.get());

    lightDisplay.shader = mkU<Shader>(device, "solid.vert", "solid.frag");
    pipelineFactory.reset();
    pipelineFactory.addVertexInputBinding({0, sizeof(glm::vec3), vk::VertexInputRate::eVertex}); // position
    pipelineFactory.parseShader("solid.vert", "solid.frag", layoutCache, false);
    pipelineFactory.getColorBlending().attachmentCount = 1;
    lightDisplay.pipeline = pipelineFactory.buildPipeline(offscreenRenderPass.get(), 3, lightDisplay.shader.get());

    solidPlane.shader = mkU<Shader>(device, "plane.vert", "solid.frag");
    pipelineFactory.reset();
    pipelineFactory.parseShader("plane.vert", "solid.frag", layoutCache, false);
    pipelineFactory.getColorBlending().attachmentCount = 1;
    solidPlane.pipeline = pipelineFactory.buildPipeline(offscreenRenderPass.get(), 3, solidPlane.shader.get());

    gridPlane.shader = mkU<Shader>(device, "grid_plane.vert", "grid_plane.frag");
    pipelineFactory.reset();
    pipelineFactory.parseShader("grid_plane.vert", "grid_plane.frag", layoutCache, false);
    pipelineFactory.getColorBlending().attachmentCount = 1;
    gridPlane.pipeline = pipelineFactory.buildPipeline(offscreenRenderPass.get(), 3, gridPlane.shader.get());

    // skinning shader
    skinning.shader = mkU<Shader>(device, "skinning.comp");
    pipelineFactory.reset();
    pipelineFactory.parseComputeShader("skinning.comp", layoutCache);
    skinning.pipeline = pipelineFactory.buildComputePipeline(skinning.shader.get());
}

void VulkanApp::createPostProcess()
{
    auto device = vulkanContext.device.logicalDevice;

    postProcessManager = mkU<PostProcessManager>(vulkanContext.device.logicalDevice, gBuffer->getResolution(), bufferFactory, renderPassBuilder);
    passAndClear.shader = mkU<Shader>(device, "passthrough.vert", "passthrough_depth.frag");
    pipelineFactory.reset();
    pipelineFactory.parseShader("passthrough.vert", "passthrough_depth.frag", layoutCache, false);
    pipelineFactory.getDepthStencil().depthWriteEnable = VK_FALSE;
    pipelineFactory.getColorBlending().attachmentCount = 1;
    passAndClear.pipeline = pipelineFactory.buildPipeline(&postProcessManager->getPingPongRenderPass(), 0, passAndClear.shader.get());
    auto fxaa = mkS<FXAA>(vulkanContext.device.logicalDevice, "fxaa.frag",
                          pipelineFactory, layoutCache, &postProcessManager->getPingPongRenderPass());
    auto vignette = mkS<Vignette>(vulkanContext.device.logicalDevice, "vignette.frag",
                                  pipelineFactory, layoutCache, &postProcessManager->getPingPongRenderPass());
    auto sharpen = mkS<Sharpen>(vulkanContext.device.logicalDevice, "sharpen.frag",
                                pipelineFactory, layoutCache, &postProcessManager->getPingPongRenderPass());
    auto filmGrain = mkS<FilmGrain>(vulkanContext.device.logicalDevice, "filmgrain.frag",
                                    pipelineFactory, layoutCache, &postProcessManager->getPingPongRenderPass());
    auto chromaticAberration = mkS<ChromaticAberration>(vulkanContext.device.logicalDevice, "chromatic_aberration.frag",
                                                        pipelineFactory, layoutCache, &postProcessManager->getPingPongRenderPass());
    postProcessManager->addPostProcess(fxaa);
    postProcessManager->addPostProcess(vignette);
    postProcessManager->addPostProcess(sharpen);
    postProcessManager->addPostProcess(filmGrain);
    postProcessManager->addPostProcess(chromaticAberration);
    postProcessManager->activatePostProcess(0);
    auto reinhard = mkU<PostProcess>("Reinhard", vulkanContext.device.logicalDevice, "reinhard.frag",
                                     pipelineFactory, layoutCache, &postProcessManager->getPingPongRenderPass());
    auto neutral = mkU<PostProcess>("Khronos PBR Neutral", vulkanContext.device.logicalDevice, "pbr_neutral.frag",
                                    pipelineFactory, layoutCache, &postProcessManager->getPingPongRenderPass());
    auto aces = mkU<PostProcess>("ACES", vulkanContext.device.logicalDevice, "aces.frag",
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

    const auto resolution = vk::Extent2D(1280, 720);
    depthBuffer = bufferFactory.createAttachment(vk::Format::eD32Sfloat, {resolution.width, resolution.height, 1},
                                                   vk::ImageUsageFlagBits::eDepthStencilAttachment
                                                   | vk::ImageUsageFlagBits::eInputAttachment | vk::ImageUsageFlagBits::eSampled,
                                                   vk::ImageAspectFlagBits::eDepth);

    createRenderPasses();

    createPipelines();

    vulkanContext.swapChain->createFramebuffers(displayRenderPass->getRenderPass());

    createGbuffer(resolution, offscreenRenderPass->getRenderPass());

    syncer.create(device);

    const auto flags = VMA_ALLOCATION_CREATE_MAPPED_BIT |
                                                                VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        frames.emplace_back(device, *graphicsCommandPool, syncer);

        computeCommandBuffers.push_back(graphicsCommandPool->createCommandBuffer());
        computeSemaphores.push_back(syncer.createSemaphore());
        computeFences.push_back(syncer.createFence(false));

        cameraBuffers.emplace_back(bufferFactory.createBuffer(sizeof(CameraMatrices),
                                                              vk::BufferUsageFlagBits::eUniformBuffer,
                                                              flags));

        sphereLightBuffers.emplace_back(bufferFactory.createBuffer(sizeof(SphereLightShader) * MAX_LIGHTS,
                                                              vk::BufferUsageFlagBits::eStorageBuffer,
                                                              flags));

        tubeLightsBuffers.emplace_back(bufferFactory.createBuffer(sizeof(TubeLightShader) * MAX_LIGHTS,
                                                                   vk::BufferUsageFlagBits::eStorageBuffer,
                                                                   flags));

        rectangleLightBuffers.emplace_back(bufferFactory.createBuffer(sizeof(RectangleLightShader) * MAX_LIGHTS,
                                                                      vk::BufferUsageFlagBits::eStorageBuffer,
                                                                      flags));

        allocators.emplace_back(vulkanContext.device.logicalDevice);
    }

    envMap.create(bufferFactory, *graphicsCommandPool, vulkanContext.queueManager,
                  "../resources/envmaps/artist_workshop/artist_workshop.dds"
//                  "../resources/envmaps/metro_noord/metro_noord.dds"
                  );

    createPostProcess();

    float pattern[] = {
            0, 32,  8, 40,  2, 34, 10, 42,
            48, 16, 56, 24, 50, 18, 58, 26,
            12, 44,  4, 36, 14, 46,  6, 38,
            60, 28, 52, 20, 62, 30, 54, 22,
            3, 35, 11, 43,  1, 33,  9, 41,
            51, 19, 59, 27, 49, 17, 57, 25,
            15, 47,  7, 39, 13, 45,  5, 37,
            63, 31, 55, 23, 61, 29, 53, 21 };
    for(float & i : pattern)
        i /= 64.0f;
    bayerMatrix = bufferFactory.createBuffer(sizeof(pattern), vk::BufferUsageFlagBits::eUniformBuffer, flags);
    bayerMatrix->fill(pattern);

    samplerSuite = mkU<SamplerSuite>(vulkanContext.device.logicalDevice);
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
                                           bufferFactory, filePath.c_str(), MAX_FRAMES_IN_FLIGHT);
        std::lock_guard lock(modelMutex);
        models.push_back(std::move(newModel));
        GLTFLoader::setLoadPercent(1.f);
    });
    t.detach();
}

void VulkanApp::recordOffscreenBuffer(vk::CommandBuffer commandBuffer, DescriptorAllocator &allocator)
{
    offscreenRenderPass->setFramebuffer(gBuffer->framebuffer);
    offscreenRenderPass->setRenderAreaExtent(gBuffer->getResolution());
    offscreenRenderPass->clear(0.f, 0.f, 0.f, 0.f);
    offscreenRenderPass->begin(commandBuffer);

    vk::DescriptorSetLayout layout;
    vk::DescriptorSet cameraSet;
    auto bufferInfo = cameraBuffers[currentFrame]->getDescriptorInfo();

    DescriptorBuilder::beginSet(&layoutCache, &allocator)
            .bindBuffer(0, &bufferInfo, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex)
            .build(cameraSet, layout);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, gBufferPS.pipeline->getPipelineLayout(),
                                      0, {cameraSet}, nullptr);

    std::unique_lock lock(modelMutex);
    if (!models.empty())
    {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, gBufferPS.pipeline->getPipeline());
        auto &model = models.back();
        // get image infos of all model textures
        std::vector<vk::DescriptorImageInfo> imageInfos = model->getDescriptorImageInfo();
        vk::DescriptorSet samplerSet;

        if (!imageInfos.empty()) // pipeline still expects uv though
        {
            const int textureCount = 80;
            if (imageInfos.size() > textureCount) throw std::runtime_error("Too many textures");
            auto bmi = bayerMatrix->getDescriptorInfo();
            DescriptorBuilder::beginSet(&layoutCache, &allocator)
                    .bindImage(0, imageInfos,
                               textureCount, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
                    .bindBuffer(1, &bmi, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment)
                    .build(samplerSet, layout);

            commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, gBufferPS.pipeline->getPipelineLayout(),
                                             1, {samplerSet}, nullptr);
        }

        model->getRoot()->draw(commandBuffer, *gBufferPS.pipeline);
    }
    lock.unlock();

    offscreenRenderPass->nextSubpass(); // composition pass

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, lighting.pipeline->getPipeline());
    lightingParameters.cubeMapRotation = glm::rotate(glm::radians(cubeMapRotation), glm::vec3(0.f, 1.f, 0.f));
    lightingParameters.pointLightCount = 0;
    lightingParameters.rectangleLightCount = static_cast<int>(rectangleLights.size());
    lightingParameters.sphereLightCount = static_cast<int>(sphereLights.size());
    lightingParameters.tubeLightCount = static_cast<int>(tubeLights.size());

    lighting.pipeline->setPushConstant(commandBuffer, &lightingParameters, sizeof(LightingParameters), 0, vk::ShaderStageFlagBits::eFragment);

    std::vector<std::vector<vk::DescriptorImageInfo>> gBufferInfo =
    {
        {gBuffer->getAttachment(GBufferAttachmentType::ALBEDO)->getDescriptorInfo(nullptr)},
        {gBuffer->getAttachment(GBufferAttachmentType::NORMAL)->getDescriptorInfo(nullptr)},
        {gBuffer->getAttachment(GBufferAttachmentType::METAL_ROUGHNESS_AO)->getDescriptorInfo(nullptr)},
        {gBuffer->getAttachment(GBufferAttachmentType::EMISSIVE)->getDescriptorInfo(nullptr)},
        {depthBuffer->getDescriptorInfo(samplerSuite->getNearestSampler())}
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
    std::vector<std::vector<vk::DescriptorImageInfo>> cubeMapInfos =
    {
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

    if (sphereLights.size() > MAX_LIGHTS) throw std::runtime_error("Too many sphere lights");

    auto sphereLightBufferInfo = sphereLightBuffers[currentFrame]->getDescriptorInfo();
    auto tubeLightBufferInfo = tubeLightsBuffers[currentFrame]->getDescriptorInfo();
    auto rectangleLightBufferInfo = rectangleLightBuffers[currentFrame]->getDescriptorInfo();
    vk::DescriptorSet lightSet;
    DescriptorBuilder::beginSet(&layoutCache, &allocator)
            .bindBuffer(0, &sphereLightBufferInfo, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eFragment)
            .bindBuffer(1, &tubeLightBufferInfo, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eFragment)
            .bindBuffer(2, &rectangleLightBufferInfo, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eFragment)
            .build(lightSet, layout);

    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, lighting.pipeline->getPipelineLayout(),
                                     0, {cameraSet, inputSet, iblSamplerSet, lightSet}, nullptr);

    commandBuffer.draw(3, 1, 0, 0);

    offscreenRenderPass->nextSubpass(); // cube map pass

    if (drawBackground)
    {
        vk::DescriptorSet cubeSamplerSet;
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, cubeMap.pipeline->getPipeline());
        DescriptorBuilder::beginSet(&layoutCache, &allocator)
                .bindImage(0, {envMap.cubeMap.texture->getDescriptorInfo()}, // env map sampler
                           1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
                .build(cubeSamplerSet, layout);

        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, cubeMap.pipeline->getPipelineLayout(),
                                         0, {cameraSet, cubeSamplerSet}, nullptr);
        auto cbmrm = glm::rotate(glm::radians(-cubeMapRotation) + 0.001f, glm::vec3(0.f, 1.f, 0.f));
        cubeMap.pipeline->setPushConstant(commandBuffer, &cbmrm, sizeof(glm::mat4), 0, vk::ShaderStageFlagBits::eVertex);
        PrimitiveDrawer::drawCube(commandBuffer);
    }

    offscreenRenderPass->nextSubpass(); // light display pass

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, lightDisplay.pipeline->getPipeline());
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, lightDisplay.pipeline->getPipelineLayout(),
                                     0, {cameraSet}, nullptr);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, lightDisplay.pipeline->getPipeline());
    for (auto &light : sphereLights)
    {
        auto translate = glm::translate(light.position);
        auto scale = glm::scale(glm::vec3(light.radius));
        PrimitiveDrawer::drawSphere(commandBuffer, *lightDisplay.pipeline, glm::vec4(light.color * light.intensity, 1.f), translate * scale);
    }
    for (auto &light : tubeLights)
    {
        auto translate = glm::translate(light.position);
        auto rotate = glm::mat4_cast(light.rotation);
        auto scale = glm::scale(glm::vec3(light.length + light.radius * 2, light.radius, light.radius));
        PrimitiveDrawer::drawCylinder(commandBuffer, *lightDisplay.pipeline, glm::vec4(light.color * light.intensity, 1.f), translate * rotate * scale);
    }
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, solidPlane.pipeline->getPipeline());
    for (auto &light : rectangleLights)
    {
        auto translate = glm::translate(light.position);
        auto rotate = glm::mat4_cast(light.rotation);
        auto scale = glm::scale(glm::vec3(light.size, 1.f));
        auto model = translate * rotate * scale;
        auto color = glm::vec4(light.color * light.intensity, 1.f);
        solidPlane.pipeline->setPushConstant(commandBuffer, &model, sizeof(glm::mat4), 0, vk::ShaderStageFlagBits::eVertex);
        solidPlane.pipeline->setPushConstant(commandBuffer, &color, sizeof(glm::vec4), sizeof(glm::mat4), vk::ShaderStageFlagBits::eFragment);
        commandBuffer.draw(6, 1, 0, 0);
    }

    if (drawGrid)
    {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, gridPlane.pipeline->getPipeline());
        auto bmi = bayerMatrix->getDescriptorInfo();
        vk::DescriptorSet bayerSet;
        DescriptorBuilder::beginSet(&layoutCache, &allocator)
                .bindBuffer(0, &bmi, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment)
                .build(bayerSet, layout);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, gridPlane.pipeline->getPipelineLayout(),
                                         0, {cameraSet, bayerSet}, nullptr);
        gridPlane.pipeline->setPushConstant(commandBuffer, &gridScale, sizeof(float), 0, vk::ShaderStageFlagBits::eFragment);
        commandBuffer.draw(6, 1, 0, 0);
    }

    offscreenRenderPass->end();
}

static bool inRenderWindow;

void VulkanApp::recordCommandBuffer(FrameBuffer *framebuffer)
{
    // TODO: multi thread secondary command buffer recording
    auto &frame = frames[currentFrame];
    auto primaryCommandBuffer = frame.commandBuffer;
    const auto gBufferResolution = gBuffer->getResolution();

    updateCameraBuffer();
    updateLightBuffers();

    //// BEGIN RECORDING COMMAND BUFFER
    frame.begin(); // begins the frames command buffer

    auto &allocator = allocators[currentFrame];
    allocator.resetPools();
    // allocate one time command buffer primary
    auto compute = computeCommandBuffers[currentFrame];
    compute.reset();
    vk::CommandBufferBeginInfo beginInfo{};
    compute.begin(beginInfo);
    std::unique_lock lock(modelMutex);
        if (!models.empty())
        {
            auto &model = models.back();
            model->updateAnimation((float)glfwGetTime(), currentFrame);
            model->getRoot()->updateJointTransforms(currentFrame);

            compute.bindPipeline(vk::PipelineBindPoint::eCompute, skinning.pipeline->getPipeline());

            model->getRoot()->skin(compute, *skinning.pipeline, layoutCache, allocator, currentFrame);
        }
    lock.unlock();
    compute.end();

    ScreenUtils::setViewport(primaryCommandBuffer, gBufferResolution.width, gBufferResolution.height);
    ScreenUtils::setScissor(primaryCommandBuffer, gBufferResolution);
    recordOffscreenBuffer(primaryCommandBuffer, allocator);

    // output already transitioned by render pass
    auto &outputAttachment = gBuffer->attachments.back();
    auto outputAttachmentInfo = outputAttachment->getDescriptorInfo(samplerSuite->getNearestSampler());
    auto bmi = bayerMatrix->getDescriptorInfo();
    vk::DescriptorSetLayout lt; vk::DescriptorSet ditherSet;
    DescriptorBuilder::beginSet(&layoutCache, &allocator)
            .bindBuffer(0, &bmi, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment)
            .build(ditherSet, lt);
    postProcessManager->tonemap(primaryCommandBuffer, layoutCache, allocator, outputAttachmentInfo, ditherSet);

    //// CLEAR COLOR
    auto &ppr = postProcessManager->getPingPongRenderPass();
    ppr.setFramebuffer(postProcessManager->getFramebuffer(1));
    ppr.setRenderAreaExtent(gBuffer->getResolution());
    ppr.clear(0.f, 0.f, 0.f, 1.f);
    ppr.begin(primaryCommandBuffer);
    primaryCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, passAndClear.pipeline->getPipeline());
    vk::DescriptorSet pprSamplerSet; vk::DescriptorSetLayout layout;
    DescriptorBuilder::beginSet(&layoutCache, &allocator)
            .bindImage(0, {postProcessManager->getAttachment(0)->getDescriptorInfo(samplerSuite->getNearestSampler())},
                       1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
            .build(pprSamplerSet, layout);
    primaryCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, passAndClear.pipeline->getPipelineLayout(),
                                            0, {pprSamplerSet}, nullptr);
    passAndClear.pipeline->setPushConstant(primaryCommandBuffer, &clearColor, sizeof(glm::vec4), 0, vk::ShaderStageFlagBits::eFragment);
    primaryCommandBuffer.draw(3, 1, 0, 0);
    ppr.end();
    //// END CLEAR COLOR

    postProcessManager->render(1, (float)glfwGetTime(), primaryCommandBuffer, layoutCache, allocator, samplerSuite->getNearestSampler());

    // final post process buffer already in shader read layout from render pass

    displayRenderPass->setFramebuffer(framebuffer->framebuffer);
    displayRenderPass->setRenderAreaExtent(vulkanContext.swapChain->getExtent());
    displayRenderPass->clear(0.f, 0.f, 0.f, 1.0f);
    displayRenderPass->begin(primaryCommandBuffer);

    primaryCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pass.pipeline->getPipeline());
    primaryCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pass.pipeline->getPipeline());
    vk::DescriptorSet samplerSet;
    DescriptorBuilder::beginSet(&layoutCache, &allocator)
            .bindImage(0, {postProcessManager->getCurrentAttachment()->getDescriptorInfo(samplerSuite->getNearestSampler())}, // output attachment sampler
                       1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
            .build(samplerSet, layout);

    const auto gRes = gBuffer->getResolution();
    inRenderWindow = ui->renderImage(samplerSet, ImVec2(gRes.width, gRes.height));

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

    vk::PipelineStageFlags computeFlag = vk::PipelineStageFlagBits::eComputeShader;

    // submit the compute job first
    vk::SubmitInfo computeSubmitInfo;
    computeSubmitInfo.commandBufferCount = 1;
    computeSubmitInfo.pCommandBuffers = &computeCommandBuffers[currentFrame];
    computeSubmitInfo.signalSemaphoreCount = 1;
    computeSubmitInfo.pSignalSemaphores = &computeSemaphores[currentFrame];
    computeSubmitInfo.pWaitDstStageMask = &computeFlag;

    queueManager.submitGraphics(computeSubmitInfo, VK_NULL_HANDLE);

    // lots of information about syncing in frame.getSubmitInfo() struct
    frame.submit(queueManager, {computeSemaphores[currentFrame]}, {vk::PipelineStageFlagBits::eVertexInput});

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
    auto extent = gBuffer->getResolution();
    float aspect = extent.width / static_cast<float>(extent.height);
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
    InputProcesser::setUserPointer(nullptr);
    if (inRenderWindow)
    {
        InputProcesser::setUserPointer(&camera);
        if ((InputProcesser::getButton(GLFW_MOUSE_BUTTON_LEFT) ||
             InputProcesser::getButton(GLFW_MOUSE_BUTTON_RIGHT)) && !InputProcesser::getButton(GLFW_KEY_LEFT_ALT))
        {
            InputProcesser::disableCursor();
        }
        else if (InputProcesser::getCursorState() == GLFW_CURSOR_DISABLED || InputProcesser::getButton(GLFW_KEY_LEFT_ALT))
        {
            InputProcesser::setUserPointer(nullptr);
            InputProcesser::enableCursor();
        }
    }
    else if (InputProcesser::getCursorState() == GLFW_CURSOR_DISABLED)
    {
        InputProcesser::enableCursor();
    }
}

void VulkanApp::updateLightBuffers()
{
    auto sphereLightBuffer = static_cast<SphereLightShader*>(sphereLightBuffers[currentFrame]->getPointer());
    for (int i = 0; i < sphereLights.size(); i++)
    {
        auto &sl = sphereLights[i];
        sphereLightBuffer[i] = {sl.position, sl.color * sl.intensity, sl.radius};
    }
    auto tubeLightBuffer = static_cast<TubeLightShader*>(tubeLightsBuffers[currentFrame]->getPointer());
    for (int i = 0; i < tubeLights.size(); i++)
    {
        auto &tl = tubeLights[i];
        TubeLightShader light;
        light.color = tl.color * tl.intensity;
        light.radius = glm::max(tl.radius, 0.001f);

        auto &rm = tl.rotation;
        auto tm = glm::translate(tl.position);

        auto px = glm::vec3(glm::abs(tl.length), 0.f, 0.f);

        light.position1 = glm::vec3(tm * (rm * glm::vec4(px, 1.f)));
        light.position2 = glm::vec3(tm * (rm * glm::vec4(-px, 1.f)));

        tubeLightBuffer[i] = light;
    }
    auto rectangleLightBuffer = static_cast<RectangleLightShader*>(rectangleLightBuffers[currentFrame]->getPointer());
    for (int i = 0; i < rectangleLights.size(); i++)
    {
        auto &rl = rectangleLights[i];
        RectangleLightShader light;
        light.position = rl.position;
        light.color = rl.color * rl.intensity;

        auto &rm = rl.rotation;
        light.right = glm::vec3(rm * glm::vec4(glm::abs(rl.size.x), 0.f, 0.f, 0.f));
        light.up = glm::vec3(rm * glm::vec4(0.f, glm::abs(rl.size.y), 0.f, 0.f));

        rectangleLightBuffer[i] = light;
    }
}

void VulkanApp::attemptToDeleteOldModel()
{
    std::unique_lock lock(modelMutex, std::try_to_lock);
    if (!lock.owns_lock()) return;
    if (models.size() > 1)
    {
        bool canDelete = true;
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            auto result = vulkanContext.device.logicalDevice.getFenceStatus(frames[i].inFlightFence);
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

MenuPayloads VulkanApp::getPartialMenuPayload()
{
    MenuPayloads m{};
    m.deviceName = &vulkanContext.device.vkbDevice.physical_device.name;
    m.postProcessManager = postProcessManager.get();
    m.camera = &camera;
    m.visualMenuPayload.lightingParameters = &lightingParameters;
    m.visualMenuPayload.cubeMapRotation = &cubeMapRotation;
    m.visualMenuPayload.clearColor = &clearColor;
    m.visualMenuPayload.drawBackground = &drawBackground;
    m.visualMenuPayload.drawGrid = &drawGrid;
    m.visualMenuPayload.gridScale = &gridScale;
    m.lightsList.sphereLights = &sphereLights;
    m.lightsList.tubeLights = &tubeLights;
    m.lightsList.rectangleLights = &rectangleLights;
    return m;
}
