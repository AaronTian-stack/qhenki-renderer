#include "renderpass.h"
#include <stdexcept>

RenderPass::RenderPass()
{
    reset();
}

void RenderPass::create(vk::Device device)
{
    this->device = device;
    renderPass = device.createRenderPass(renderPassInfo);
}

void RenderPass::destroy()
{
    device.destroyRenderPass(renderPass);
}

void RenderPass::reset()
{
    colorAttachment.format = vk::Format::eA8B8G8R8UintPack32; // need to provide swapchain format
    colorAttachment.samples = vk::SampleCountFlagBits::e1;
    colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
    colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

    colorAttachmentRef.attachment = 0; // index in pAttachments array. used in the shader! (layout(location = 0) out vec4 outColor)
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal; // TODO: allow for more attachments

    // TODO: abstract attachment setup
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment; // TODO: allow for more attachments
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
}

void RenderPass::setColorAttachmentFormat(vk::Format format)
{
    colorAttachment.format = format;
}

void RenderPass::begin(vk::CommandBuffer commandBuffer)
{
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.renderArea.offset = vk::Offset2D{0, 0};

    // also need renderPassInfo.framebuffer and renderPassInfo.renderArea.extent

    // set up the command buffer for this render pass
    commandBuffer.beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);
    currentCommandBuffer = commandBuffer;
}

void RenderPass::clear(float r, float g, float b, float a)
{
    clearColor = vk::ClearColorValue(std::array<float, 4>{r, g, b, a});
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearColor;
}

void RenderPass::setFramebuffer(VkFramebuffer buffer)
{
    renderPassBeginInfo.framebuffer = buffer;
}

void RenderPass::setRenderAreaExtent(VkExtent2D extent)
{
    renderPassBeginInfo.renderArea.extent = extent;
}

void RenderPass::end()
{
    if (!currentCommandBuffer.has_value())
        throw std::runtime_error("RenderPass::end() called without begin()!");

    currentCommandBuffer->endRenderPass();
    currentCommandBuffer = std::nullopt;
}
