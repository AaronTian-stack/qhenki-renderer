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

    pipeline1->destroy();
    shader1->destroy(); // shader modules must be destroyed after pipeline

    pipeline2->destroy();
    shader2->destroy();

    renderPass.destroy();

    buffer->destroy();
    bufferFactory.destroy();
}

void PathTracerApp::create(Window &window)
{
    vulkanContext.create(window);

    auto device = vulkanContext.device.logicalDevice;

    bufferFactory.create(vulkanContext);

    // create render pass
    renderPass.setColorAttachmentFormat(vulkanContext.swapChain.getFormat());
    //// CALL CREATE LAST! OR ELSE STUFF WILL BE BROKEN!
    renderPass.create(device);

    shader1 = mkU<Shader>(device, "pathtrace_vert.spv", "pathtrace_frag.spv");
    shader2 = mkU<Shader>(device, "tri_vert.spv", "tri_frag.spv");

    pipelineFactory.addPushConstant(sizeof(float));

    pipeline1 = pipelineFactory.buildPipeline(device, &renderPass, shader1.get());

    //// VERTEX INPUT
    struct Vertex {
        glm::vec2 pos;
        glm::vec3 color;
    };
    const std::vector<Vertex> vertices = {
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
    };

    buffer = bufferFactory.createBuffer(vertices.size() * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    buffer->fill(vertices.data(), vertices.size() * sizeof(Vertex));

    pipelineFactory.reset();
    pipelineFactory.addVertexInputBinding({0, sizeof(Vertex), vk::VertexInputRate::eVertex});
    pipelineFactory.addVertexInputAttribute({0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, pos)});
    pipelineFactory.addVertexInputAttribute({1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)});
    //// END VERTEX INPUT

    pipeline2 = pipelineFactory.buildPipeline(device, &renderPass, shader2.get());

    vulkanContext.swapChain.createFramebuffers(renderPass);

    commandPool.create(device, vulkanContext.device.queueFamilyIndices.graphicsFamily.value());
    commandPool.createCommandBuffer("main");

    syncer.create(device);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        frames.emplace_back(device, commandPool, syncer);

    //gltfLoad.load();
}

void PathTracerApp::recordCommandBuffer(VkFramebuffer framebuffer)
{
    auto &frame = frames[currentFrame];
    auto commandBuffer = frame.commandBuffer;
    auto swapChainExtent = vulkanContext.swapChain.getExtent();

    //// BEGIN RECORDING COMMAND BUFFER
    frame.begin(); // begins the frames command buffer. for secondary command buffer, need to modify the beginInfo struct

    //// BEGIN RENDER PASS
    renderPass.setFramebuffer(framebuffer);
    renderPass.setRenderAreaExtent(swapChainExtent);
    renderPass.clear(ui->clearColor[0], ui->clearColor[1], ui->clearColor[2], 1.0f);
    renderPass.begin(commandBuffer);

    auto pipeline = ui->currentShaderIndex == 0 ? pipeline1.get() : pipeline2.get();
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->getGraphicsPipeline());
    pipeline1->setPushConstant(commandBuffer, (float)glfwGetTime());

    ScreenUtils::setViewport(commandBuffer, swapChainExtent.width, swapChainExtent.height);
    ScreenUtils::setScissor(commandBuffer, swapChainExtent);

    if (ui->currentShaderIndex == 1)
        buffer->bind(commandBuffer);

    //// DRAW COMMAND(S)
    commandBuffer.draw(3, 1, 0, 0);
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
