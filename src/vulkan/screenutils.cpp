#include "screenutils.h"

void ScreenUtils::setViewport(vk::CommandBuffer commandBuffer, uint32_t width, uint32_t height)
{
    vk::Viewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(width);
    viewport.height = static_cast<float>(height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    // see vkCmdSetViewport
    commandBuffer.setViewport(0, 1, &viewport);
}

void ScreenUtils::setScissor(vk::CommandBuffer commandBuffer, vk::Extent2D extent)
{
    vk::Rect2D scissor{};
    scissor.offset = vk::Offset2D{0, 0};
    scissor.extent = extent;
    // see vkCmdSetScissor
    commandBuffer.setScissor(0, 1, &scissor);
}
