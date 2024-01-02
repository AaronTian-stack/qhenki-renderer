#pragma once

#include <vulkan/vulkan.h>
#include "../disposable.h"

class RenderPass : public Disposable
{
private:
    VkCommandBuffer currentCommandBuffer = VK_NULL_HANDLE;

    VkRenderPass renderPass;

    VkRenderPassCreateInfo renderPassInfo{};
    VkAttachmentDescription colorAttachment{}; // TODO: add more attachment options
    VkAttachmentReference colorAttachmentRef{}; // one ref per attachment... pair them in a container

    VkSubpassDescription subpass{}; // the sub-passes are specified in this

    VkClearValue clearColor{};
    VkRenderPassBeginInfo renderPassBeginInfo{};

public:
    RenderPass();
    void create(VkDevice device);
    void reset();
    void dispose() override;

    VkRenderPass getRenderPass() { return renderPass; }
    void setColorAttachmentFormat(VkFormat format);

    void clear(float r, float g, float b, float a);
    void begin(VkCommandBuffer commandBuffer);
    void end();
    void setFramebuffer(VkFramebuffer buffer);
    void setRenderAreaExtent(VkExtent2D extent);
};
