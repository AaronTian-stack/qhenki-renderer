#include "renderpass.h"
#include <stdexcept>

RenderPass::RenderPass(vk::Device device, vk::RenderPass renderPass) : Destroyable(device), renderPass(renderPass)
{}

void RenderPass::destroy()
{
    device.destroyRenderPass(renderPass);
}

void RenderPass::begin(vk::CommandBuffer commandBuffer)
{
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.renderArea.offset = vk::Offset2D{0, 0};

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

void RenderPass::setFramebuffer(vk::Framebuffer buffer)
{
    renderPassBeginInfo.framebuffer = buffer;
}

void RenderPass::setRenderAreaExtent(vk::Extent2D extent)
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
