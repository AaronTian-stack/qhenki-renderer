#include <limits>
#include "swapchain.h"

SwapChain::SwapChain(vkb::Swapchain vkbSwapchain)
: vkbSwapchain(vkbSwapchain), swapChain(vkbSwapchain.swapchain)
{}

vk::Format SwapChain::getFormat() const
{
    return vk::Format(vkbSwapchain.image_format);
}

vk::Extent2D SwapChain::getExtent() const
{
    return {vkbSwapchain.extent.width, vkbSwapchain.extent.height};
}

vk::Framebuffer SwapChain::nextImage(vk::Semaphore imageAvailable)
{
    auto device = vk::Device(this->vkbSwapchain.device);
    auto result = device.acquireNextImageKHR(swapChain, UINT64_MAX,
                                             imageAvailable, VK_NULL_HANDLE);
    imageIndex = result.value;
    return frameBuffers[imageIndex].framebuffer;
}

void SwapChain::createFramebuffers(vk::RenderPass renderPass, vk::ImageView depthImageView)
{
    auto device = vk::Device(this->vkbSwapchain.device);
    auto extent = getExtent();
    auto images = vkbSwapchain.get_images().value();
    auto imageViews = vkbSwapchain.get_image_views().value();
    frameBuffers.reserve(imageViews.size());
    for (size_t i = 0; i < imageViews.size(); i++)
    {
        vk::ImageView attachments[] = {imageViews[i], depthImageView};
        int attachmentCount = depthImageView ? 2 : 1;
        vk::FramebufferCreateInfo createInfo(
                vk::FramebufferCreateFlags(),
                renderPass,
                attachmentCount,
                attachments,
                extent.width,
                extent.height,
                1);
        auto framebuffer = device.createFramebuffer(createInfo);
        auto attachment = FrameBufferAttachment(images[i], imageViews[i], getFormat());
        auto fb = FrameBuffer(device, framebuffer, {attachment});
        frameBuffers.push_back(fb);
    }
}

void SwapChain::destroy()
{
    auto device = vk::Device(this->vkbSwapchain.device);
    for (auto &frameBuffer : frameBuffers)
    {
        device.destroy(frameBuffer.framebuffer);
        for (auto &attachment : frameBuffer.attachments)
        {
            device.destroyImageView(attachment.imageView);
        }
    }
    // images are destroyed with the swapchain
    vkb::destroy_swapchain(vkbSwapchain);
}
