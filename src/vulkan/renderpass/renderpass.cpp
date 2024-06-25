#include "renderpass.h"
#include <stdexcept>

RenderPass::RenderPass(vk::Device device, vk::RenderPass renderPass, std::vector<vk::Format> formats)
: Destroyable(device), renderPass(renderPass), formats(formats)
{
    // determine clear values based off formats
    for (auto &format : formats)
    {
        if (format == vk::Format::eD32Sfloat)
        {
            clearValues.emplace_back(vk::ClearDepthStencilValue(1.0f, 0));
        }
        else
        {
            clearValues.emplace_back(vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}));
        }
    }
}

void RenderPass::destroy()
{
    device.destroyRenderPass(renderPass);
}

void RenderPass::begin(vk::CommandBuffer commandBuffer)
{
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.renderArea.offset = vk::Offset2D{0, 0};

    // set up the command buffer for this render pass
    // command buffers are tied to render pass. consider using secondary command buffers
    commandBuffer.beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);
    currentCommandBuffer = commandBuffer;
}

void RenderPass::nextSubpass()
{
    currentCommandBuffer->nextSubpass(vk::SubpassContents::eInline);
}

void RenderPass::clear(float r, float g, float b, float a)
{
    for (int i = 0; i < clearValues.size(); i++)
    {
        if (formats[i] == vk::Format::eD32Sfloat)
        {
            clearValues[i] = vk::ClearDepthStencilValue(1.0f, 0);
        }
        else
        {
            clearValues[i] = vk::ClearColorValue(std::array<float, 4>{r, g, b, a});
        }
    }
    renderPassBeginInfo.clearValueCount = clearValues.size();
    renderPassBeginInfo.pClearValues = clearValues.data();
}

void RenderPass::doNotClear()
{
    renderPassBeginInfo.clearValueCount = 0;
    renderPassBeginInfo.pClearValues = nullptr;
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
