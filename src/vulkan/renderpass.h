#pragma once

#include <vulkan/vulkan.hpp>
#include "destroyable.h"

class RenderPass : public Destroyable
{
private:
    std::optional<vk::CommandBuffer> currentCommandBuffer;

    vk::RenderPass renderPass;

    vk::RenderPassCreateInfo renderPassInfo{};
    vk::AttachmentDescription colorAttachment{}; // TODO: add more attachment options
    vk::AttachmentReference colorAttachmentRef{}; // one ref per attachment... pair them in a container

    vk::SubpassDescription subpass{}; // the sub-passes are specified in this

    vk::ClearValue clearColor{};
    vk::RenderPassBeginInfo renderPassBeginInfo{};

public:
    RenderPass();
    void create(vk::Device device) override;
    void reset();
    void destroy() override;

    vk::RenderPass getRenderPass() { return renderPass; }
    void setColorAttachmentFormat(vk::Format format);

    void clear(float r, float g, float b, float a);
    void begin(vk::CommandBuffer commandBuffer);
    void end();
    void setFramebuffer(vk::Framebuffer buffer);
    void setRenderAreaExtent(vk::Extent2D extent);
};
