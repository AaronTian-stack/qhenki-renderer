#include "pathtracerapp.h"
#include "vulkan/screenutils.h"

PathTracerApp::PathTracerApp()
{

}

PathTracerApp::~PathTracerApp()
{
    // destroy instance last
    vulkanInstance.dispose();
}

void PathTracerApp::dispose()
{
    vkDeviceWaitIdle(devicePicker.getDevice()); // wait for operations to finish before disposing

    for (auto &frame : frames)
        frame.dispose();

    syncer.dispose();
    commandPool.dispose();

    pipeline->dispose();
    shader->dispose(); // shader modules must be destroyed after pipeline
    renderPass.dispose();

    // destroy swap chain before the device
    swapChain.dispose();
    // destroy logical device
    devicePicker.dispose();
    // destroy debugger
    debugger.dispose();
}

void PathTracerApp::create(Window &window)
{
    VulkanInstance::listExtensions();

    const bool verbose = false;
    // create instance, load extensions
    vulkanInstance.create(verbose);
    // create debugger, validation layers
    debugger.create(vulkanInstance, verbose);
    window.createSurface(vulkanInstance);
    // pick physical device (with feature checking)
    devicePicker.pickPhysicalDevice(vulkanInstance, window.getSurface());
    // create queues, logical device
    devicePicker.createLogicalDevice();
    // retrieve queues
    queueManager.initQueues(devicePicker.getDevice(), devicePicker.selectedDeviceFamily());

    // create swap chain (the render pass needs the image formats)
    // TODO: maybe refactor to extract properties before the swap chain creation (so that framebuffer creation is also part of createSwapChain)
    swapChain.createSwapChain(devicePicker, window);

    // create render pass
    renderPass.setColorAttachmentFormat(swapChain.getFormat());
    //// CALL CREATE LAST! OR ELSE STUFF WILL BE BROKEN!
    renderPass.create(devicePicker.getDevice());

    shader = mkU<Shader>(devicePicker.getDevice(), "shader_vert.spv", "shader_frag.spv");

    pipeline = pipelineBuilder.buildPipeline(devicePicker.getDevice(), &renderPass, shader.get());

    swapChain.createFramebuffers(renderPass);

    commandPool.create(devicePicker.getDevice(), devicePicker.selectedDeviceFamily());
    commandPool.createCommandBuffer("main");

    syncer.create(devicePicker.getDevice());

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        frames.emplace_back(devicePicker.getDevice(), commandPool, syncer);
}

void PathTracerApp::recordCommandBuffer(VkFramebuffer framebuffer)
{
    auto &frame = frames[currentFrame];
    auto commandBuffer = frame.commandBuffer;
    auto swapChainExtent = swapChain.getExtent();

    //// BEGIN RECORDING COMMAND BUFFER
    frame.begin(); // begins the frames command buffer. for secondary command buffer, need to modify the beginInfo struct

    //// BEGIN RENDER PASS
    renderPass.setFramebuffer(framebuffer);
    renderPass.setRenderAreaExtent(swapChainExtent);
    renderPass.clear(0.0f, 0.0f, 0.0f, 1.0f);
    renderPass.begin(commandBuffer);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getGraphicsPipeline());

    ScreenUtils::setViewport(commandBuffer, swapChainExtent.width, swapChainExtent.height);
    ScreenUtils::setScissor(commandBuffer, swapChainExtent);

    //// DRAW COMMAND(S)
    pipeline->setPushConstant(commandBuffer, (float)glfwGetTime());
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

    // gets next available image, returns the corresponding framebuffer and updates internal index to next image
    auto fb = swapChain.nextImage(frame.imageAvailableSemaphore);

    vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
    recordCommandBuffer(fb);

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
    return vulkanInstance;
}

ImGuiCreateParameters PathTracerApp::getImGuiCreateParameters()
{
    ImGuiCreateParameters params{};
    params.instance = &vulkanInstance;
    params.devicePicker = &devicePicker;
    params.renderPass = &renderPass;
    params.framesInFlight = MAX_FRAMES_IN_FLIGHT;
    params.queueManager = &queueManager;
    return params;
}

CommandPool& PathTracerApp::getCommandPool()
{
    return commandPool;
}
