#include "frame.h"

Frame::Frame() {}

Frame::Frame(VkDevice device, CommandPool &pool, Syncer &sync) :
    Disposable(device),
    commandBuffer(pool.createCommandBuffer()),
    imageAvailableSemaphore(sync.createSemaphore()),
    renderFinishedSemaphore(sync.createSemaphore()),
    inFlightFence(sync.createFence(true))
{}

void Frame::create(VkDevice device, CommandPool &pool, Syncer &sync)
{
    deviceForDispose = device;
    commandBuffer = pool.createCommandBuffer();
    imageAvailableSemaphore = sync.createSemaphore();
    renderFinishedSemaphore = sync.createSemaphore();
    inFlightFence = sync.createFence(true);
}

void Frame::dispose()
{
    vkDestroySemaphore(deviceForDispose, imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(deviceForDispose, renderFinishedSemaphore, nullptr);
    vkDestroyFence(deviceForDispose, inFlightFence, nullptr);
}

void Frame::begin()
{
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional: look into this more
    beginInfo.pInheritanceInfo = nullptr; // which state to inherit from primary command buffers (only applies to secondary command buffers)

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }
}

void Frame::end()
{
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}

VkSubmitInfo Frame::getSubmitInfo()
{
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    //VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &imageAvailableSemaphore; // wait until the image has been acquired before drawing
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1; // TODO: probably needs to take in multiple command buffers at some point
    submitInfo.pCommandBuffers = &commandBuffer; // TODO: move these two lines into another function, this should not return the struct

    //VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderFinishedSemaphore; // signal this semaphore when the render is done
    return submitInfo;
}

void Frame::finish(Syncer &syncer)
{
    syncer.waitForFence(inFlightFence);
    syncer.resetFence(inFlightFence);
}
