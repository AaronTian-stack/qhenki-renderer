#include "pathtracerapp.h"
#include "vulkan/screenutils.h"
#include "glm/glm.hpp"

PathTracerApp::PathTracerApp() {}

PathTracerApp::~PathTracerApp()
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

    renderPass.destroy();

    buffer->destroy();
    indexBuffer->destroy();

    for (auto &cameraBuffer : cameraBuffers)
        cameraBuffer->destroy();

    bufferFactory.destroy();
}

void PathTracerApp::create(Window &window)
{
    vulkanContext.create(window);
    layoutCache.create(vulkanContext.device.logicalDevice);
    allocator.create(vulkanContext.device.logicalDevice);

    auto device = vulkanContext.device.logicalDevice;
    commandPool.create(device, vulkanContext.device.queueFamilyIndices.graphicsFamily.value());
    pipelineFactory.create(device);

    bufferFactory.create(vulkanContext);

    // create render pass
    renderPass.setColorAttachmentFormat(vulkanContext.swapChain.getFormat());
    //// CALL CREATE LAST! OR ELSE STUFF WILL BE BROKEN!
    renderPass.create(device);

    shader1 = mkU<Shader>(device, "pathtrace_vert.spv", "pathtrace_frag.spv");
    shader2 = mkU<Shader>(device, "tri_vert.spv", "tri_frag.spv");

    //pipelineFactory.addPushConstant(sizeof(float));
    pipelineFactory.parseFragmentShader("pathtrace_frag.spv");

    pathPipeline = pipelineFactory.buildPipeline(&renderPass, shader1.get());

    //// VERTEX INPUT
    struct Vertex {
        glm::vec2 pos;
        glm::vec3 color;
    };
    const std::vector<Vertex> vertices = {
            {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
    };
    const std::vector<uint16_t> indices = {
            0, 1, 2, 2, 3, 0
    };

    auto stagingBuffer = bufferFactory.createBuffer(
            vertices.size() * sizeof(Vertex),
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

    stagingBuffer->fill(vertices.data(), vertices.size() * sizeof(Vertex));

    buffer = bufferFactory.createBuffer(vertices.size() * sizeof(Vertex),
                                        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    stagingBuffer->copyTo(*buffer, vulkanContext.queueManager, commandPool);
    stagingBuffer->destroy();

    //buffer = bufferFactory.createBuffer(vertices.size() * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
    //buffer->fill(vertices.data(), vertices.size() * sizeof(Vertex));

    indexBuffer = bufferFactory.createBuffer(indices.size() * sizeof(uint16_t),
                                             VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
    indexBuffer->fill(indices.data(), indices.size() * sizeof(uint16_t));

    pipelineFactory.reset();

    //pipelineFactory.addDescriptorSetLayout(*shader2); // TODO: testing

    pipelineFactory.addVertexInputBinding({0, sizeof(Vertex), vk::VertexInputRate::eVertex});
    //pipelineFactory.addVertexInputAttribute({0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, pos)});
    //pipelineFactory.addVertexInputAttribute({1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)});
    pipelineFactory.parseVertexShader("tri_vert.spv", layoutCache);
    //// END VERTEX INPUT

    triPipeline = pipelineFactory.buildPipeline(&renderPass, shader2.get());

    vulkanContext.swapChain.createFramebuffers(renderPass);

    syncer.create(device);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        frames.emplace_back(device, commandPool, syncer);
        auto flags = static_cast<VmaAllocationCreateFlagBits>(VMA_ALLOCATION_CREATE_MAPPED_BIT |
                                                                                    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
        cameraBuffers.emplace_back(bufferFactory.createBuffer(sizeof(CameraMatrices),
                                                              VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                              flags));
    }

    //gltfLoad.load();
}

void PathTracerApp::recordCommandBuffer(VkFramebuffer framebuffer)
{
    auto &frame = frames[currentFrame];
    auto commandBuffer = frame.commandBuffer;
    auto swapChainExtent = vulkanContext.swapChain.getExtent();

    updateCameraBuffer();

    //// BEGIN RECORDING COMMAND BUFFER
    frame.begin(); // begins the frames command buffer. for secondary command buffer, need to modify the beginInfo struct

    //// BEGIN RENDER PASS
    renderPass.setFramebuffer(framebuffer);
    renderPass.setRenderAreaExtent(swapChainExtent);
    renderPass.clear(ui->clearColor[0], ui->clearColor[1], ui->clearColor[2], 1.0f);
    renderPass.begin(commandBuffer);

    auto pipeline = ui->currentShaderIndex == 0 ? pathPipeline.get() : triPipeline.get();
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->getGraphicsPipeline());
    pathPipeline->setPushConstant(commandBuffer, (float)glfwGetTime());

    ScreenUtils::setViewport(commandBuffer, swapChainExtent.width, swapChainExtent.height);
    ScreenUtils::setScissor(commandBuffer, swapChainExtent);

    if (ui->currentShaderIndex == 1)
    {
        buffer->bind(commandBuffer);
        indexBuffer->bind(commandBuffer);

        /*VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = cameraBuffers[currentFrame]->getBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);*/

        auto bufferInfo = cameraMatrices.getBufferInfo(*cameraBuffers[currentFrame].get());

        //allocator.resetPools();
        vk::DescriptorSet globalSet;
        vk::DescriptorSetLayout layout;
        DescriptorBuilder::begin(&layoutCache, &allocator)
                .bindBuffer(0, &bufferInfo, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex)
                .build(globalSet, layout);

        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->getPipelineLayout(), 0, {globalSet}, nullptr);
        commandBuffer.drawIndexed(6, 1, 0, 0, 0);
    }
    else
        commandBuffer.draw(3, 1, 0, 0);

    //// DRAW COMMAND(S)

    ui->end(commandBuffer);

    //// END RENDER PASS
    renderPass.end(); // ends the render pass with the command buffer given in .begin()

    //// END RECORDING COMMAND BUFFER
    frame.end(); // ends the frames command buffer
}

void PathTracerApp::render()
{
    auto &frame = frames[currentFrame];
    frame.finish(syncer); // waits for fence to be signalled (finished frame) and resets the frames inFlightFence

    auto commandBuffer = frame.commandBuffer;

    auto &swapChain = vulkanContext.swapChain;
    // gets next available image, returns the corresponding framebuffer and updates internal index to next image
    auto fb = swapChain.nextImage(frame.imageAvailableSemaphore);

    //vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
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

void PathTracerApp::resize()
{

}


ImGuiCreateParameters PathTracerApp::getImGuiCreateParameters()
{
    ImGuiCreateParameters params{};
    params.context = &vulkanContext;
    params.renderPass = &renderPass;
    params.framesInFlight = MAX_FRAMES_IN_FLIGHT;
    return params;
}

CommandPool& PathTracerApp::getCommandPool()
{
    return commandPool;
}

void PathTracerApp::updateCameraBuffer()
{
    cameraMatrices.model = glm::mat4(1.f);
    cameraMatrices.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
                                      glm::vec3(0.0f, 0.0f, 2.0f),
                                      glm::vec3(0.0f, 1.0f, 0.0f));
    cameraMatrices.proj = glm::perspective(glm::radians(45.0f),
                                           vulkanContext.swapChain.getExtent().width / (float)vulkanContext.swapChain.getExtent().height,
                                           0.1f, 10.0f);
    cameraMatrices.proj[1][1] *= -1; // flip y coordinate
    cameraBuffers[currentFrame]->fill(&cameraMatrices, sizeof(CameraMatrices));
}
