#include "pathtracerapp.h"

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

    syncer.dispose();
    commandPool.dispose();

    pipeline->dispose();
    shader->dispose(); // shader modules must be destroyed after pipeline
    renderPass.dispose();

    // destroy swap chain before the device
    swapChainManager.dispose();
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
    swapChainManager.createSwapChain(devicePicker, window);

    // create render pass
    renderPass.setDevice(devicePicker.getDevice());
    renderPass.setColorAttachmentFormat(swapChainManager.getFormat());
    //// CALL CREATE LAST! OR ELSE STUFF WILL BE BROKEN!
    renderPass.create();

    shader = mkU<Shader>(devicePicker.getDevice(), "shader_vert.spv", "shader_frag.spv");

    pipeline = pipelineBuilder.buildPipeline(devicePicker.getDevice(), &renderPass, shader.get());

    swapChainManager.createFramebuffers(renderPass);

    commandPool.create(devicePicker.getDevice(), devicePicker.selectedDeviceFamily());
    commandPool.createCommandBuffer("main");

    syncer.create(devicePicker.getDevice());
    syncer.createFence("inFlightFence", true);
    syncer.createSemaphore("imageAvailableSemaphore");
    syncer.createSemaphore("renderFinishedSemaphore");
}

void PathTracerApp::recordCommandBuffer(uint32_t imageIndex)
{
    auto commandBuffer = commandPool.getCommandBuffer("main");
    auto swapChainExtent = swapChainManager.getExtent();

    //// BEGIN RECORDING COMMAND BUFFER
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional: look into this more
    beginInfo.pInheritanceInfo = nullptr; // which state to inherit from primary command buffers (only applies to secondary command buffers)

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    //// BEGIN RENDER PASS
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass.getRenderPass();
    renderPassInfo.framebuffer = swapChainManager.getFramebuffer(imageIndex); //renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getGraphicsPipeline());

    //// SET DYNAMIC PIPELINE VALUES
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChainExtent.width);
    viewport.height = static_cast<float>(swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport); // TODO: abstract

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor); // TODO: abstract

    //// DRAW COMMAND(S)
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffer); //// END RENDER PASS

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void PathTracerApp::render()
{
    // wait for previous frame to finish
    syncer.waitForFence("inFlightFence");
    //vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
    syncer.resetFence("inFlightFence");
    //vkResetFences(device, 1, &inFlightFence);

    auto device = devicePicker.getDevice(); // TODO: remove
    auto commandBuffer = commandPool.getCommandBuffer("main"); // TODO: remove

    auto imageAvailableSemaphore = syncer.getSemaphore("imageAvailableSemaphore");
    // TODO: abstract below 3 lines into swap chain manager.
    uint32_t imageIndex;
    auto swapChain = swapChainManager.getSwapChain();
    vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
    recordCommandBuffer(imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    auto renderFinishedSemaphore = syncer.getSemaphore("renderFinishedSemaphore"); // TODO: delete
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    auto inFlightFence = syncer.getFence("inFlightFence");
    // TODO: abstract this into queue manager
    if (vkQueueSubmit(queueManager.getGraphicsQueue(), 1, &submitInfo, inFlightFence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(queueManager.getPresentQueue(), &presentInfo);
}

void PathTracerApp::resize()
{

}

VulkanInstance PathTracerApp::getVulkanInstance()
{
    return vulkanInstance;
}
