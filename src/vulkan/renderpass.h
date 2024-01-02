#pragma once

#include <vulkan/vulkan.h>
#include "../disposable.h"

class RenderPass : public Disposable
{
private:
    VkRenderPass renderPass;

    VkRenderPassCreateInfo renderPassInfo;
    VkAttachmentDescription colorAttachment; // TODO: add more attachment options
    VkAttachmentReference colorAttachmentRef; // one ref per attachment... pair them in a container

    VkSubpassDescription subpass; // the sub-passes are specified in this

public:
    RenderPass();
    void create();
    void reset();
    void dispose() override;

    VkRenderPass getRenderPass() { return renderPass; }
    void setColorAttachmentFormat(VkFormat format);
};
