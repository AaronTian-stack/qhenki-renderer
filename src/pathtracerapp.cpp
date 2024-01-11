#include "pathtracerapp.h"
#include "vulkan/screenutils.h"
#include "glm/glm.hpp"

PathTracerApp::PathTracerApp() {}

PathTracerApp::~PathTracerApp()
{
    // destroy instance last
    vulkanContext.vulkanInstance.dispose();
}

void PathTracerApp::dispose()
{
    vkDeviceWaitIdle(vulkanContext.devicePicker.getDevice()); // wait for operations to finish before disposing

    for (auto &frame : frames)
        frame.dispose();

    syncer.dispose();
    commandPool.dispose();

    pipeline1->dispose();
    shader1->dispose(); // shader modules must be destroyed after pipeline

    pipeline2->dispose();
    shader2->dispose();

    renderPass.dispose();

    buffer->dispose();
    bufferFactory.dispose();

    vulkanContext.dispose();
}

void PathTracerApp::create(Window &window)
{
    vulkanContext.create(window);
    bufferFactory.create(vulkanContext);

    auto device = vulkanContext.devicePicker.getDevice();
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
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    };
    buffer = bufferFactory.createBuffer(vertices.size() * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    buffer->fill(vertices.data(), vertices.size() * sizeof(Vertex));

    pipelineFactory.reset();
    pipelineFactory.addVertexInputBinding({0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX});
    pipelineFactory.addVertexInputAttribute({0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, pos)});
    pipelineFactory.addVertexInputAttribute({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
    pipeline2 = pipelineFactory.buildPipeline(device, &renderPass, shader2.get());
    //// END VERTEX INPUT

    vulkanContext.swapChain.createFramebuffers(renderPass);

    commandPool.create(device, vulkanContext.devicePicker.selectedDeviceFamily());
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
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getGraphicsPipeline());
    pipeline1->setPushConstant(commandBuffer, (float)glfwGetTime());


    ScreenUtils::setViewport(commandBuffer, swapChainExtent.width, swapChainExtent.height);
    ScreenUtils::setScissor(commandBuffer, swapChainExtent);

    //// DRAW COMMAND(S)
    if (pipeline == pipeline2.get())
        buffer->bind(commandBuffer);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
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

    vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
    recordCommandBuffer(fb);

    auto &queueManager = vulkanContext.queueManager;
    // lots of information about syncing in frame.getSubmitInfo() struct.
    queueManager.submitGraphics(frame.getSubmitInfo(), frame.inFlightFence); // signal fence when done

    std::vector<VkSemaphore> signalSemaphores = { frame.renderFinishedSemaphore };
    // calls the present command, waits for given semaphores
    queueManager.present(swapChain, signalSemaphores);

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void PathTracerApp::resize()
{

}

VulkanInstance& PathTracerApp::getVulkanInstance()
{
    return vulkanContext.vulkanInstance;
}

ImGuiCreateParameters PathTracerApp::getImGuiCreateParameters()
{
    ImGuiCreateParameters params{};
    params.instance = &vulkanContext.vulkanInstance;
    params.devicePicker = &vulkanContext.devicePicker;
    params.renderPass = &renderPass;
    params.framesInFlight = MAX_FRAMES_IN_FLIGHT;
    params.queueManager = &vulkanContext.queueManager;
    return params;
}

CommandPool& PathTracerApp::getCommandPool()
{
    return commandPool;
}
