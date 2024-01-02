#include "screenutils.h"

void ScreenUtils::setViewport(VkCommandBuffer commandBuffer, uint32_t width, uint32_t height)
{
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(width);
    viewport.height = static_cast<float>(height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
}

void ScreenUtils::setScissor(VkCommandBuffer commandBuffer, VkExtent2D extent)
{
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = extent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}
