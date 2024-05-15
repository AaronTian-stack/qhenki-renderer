#include "postprocessmanager.h"

PostProcessManager::PostProcessManager(vk::Device device, vk::Extent2D extent, BufferFactory &bufferFactory,
                                       RenderPassBuilder &renderPassBuilder)
: Destroyable(device)
{
    renderPassBuilder.reset();
    renderPassBuilder.addColorAttachment(vk::Format::eR8G8B8A8Unorm);
    renderPassBuilder.addSubPass({}, {}, {0}, {});
    pingPongRenderPass = renderPassBuilder.buildRenderPass();

    vk::Framebuffer fbs[2];
    for (int i = 0; i < 2; i++)
    {
        afb[i].attachment = bufferFactory.createAttachment(
                vk::Format::eR8G8B8A8Unorm,
                {extent.width, extent.height, 1},
                vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment | vk::ImageUsageFlagBits::eSampled,
                vk::ImageAspectFlagBits::eColor);
        vk::FramebufferCreateInfo createInfo(
                vk::FramebufferCreateFlags(),
                pingPongRenderPass->getRenderPass(),
                1,
                &afb[i].attachment->imageView,
                extent.width,
                extent.height,
                1);
        fbs[i] = device.createFramebuffer(createInfo);
    }
    for (int i = 0; i < 2; i++)
    {
        afb[i].framebuffer = fbs[1 - i];
    }
    currentAttachment = afb[0].attachment.get();
}

void PostProcessManager::render(vk::CommandBuffer commandBuffer)
{
    // for each effect, bind its pipeline and set whatever parameters it needs (push constant)
        // get and bind the descriptor
        // set push constant
        // draw triangle
}

void PostProcessManager::destroy()
{
    for (auto &attachment : afb)
    {
        attachment.attachment->destroy();
        device.destroyFramebuffer(attachment.framebuffer);
    }
    for (auto &postProcess : postProcesses)
    {
        postProcess->destroy();
    }
}
