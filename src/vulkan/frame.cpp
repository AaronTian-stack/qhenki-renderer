#include "frame.h"
#include "buffer/bufferfactory.h"

Frame::Frame() {}

Frame::Frame(vk::Device device, CommandPool &pool, Syncer &sync) :
        Destroyable(device),
        commandBuffer(pool.createCommandBuffer()),
        imageAvailableSemaphore(sync.createSemaphore()),
        renderFinishedSemaphore(sync.createSemaphore()),
        inFlightFence(sync.createFence(true))
{

}

void Frame::destroy()
{
    device.destroySemaphore(imageAvailableSemaphore);
    device.destroySemaphore(renderFinishedSemaphore);
    device.destroyFence(inFlightFence);
}

void Frame::begin()
{
    //beginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse; // Optional: look into this more
    beginInfo.pInheritanceInfo = nullptr; // which state to inherit from primary command buffers (only applies to secondary command buffers)

    commandBuffer.begin(beginInfo);
}

void Frame::end()
{
    commandBuffer.end();
}

vk::SubmitInfo Frame::getSubmitInfo()
{
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
