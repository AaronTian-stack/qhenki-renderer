#pragma once

#include <optional>
#include "vulkan/vulkan.hpp"
#include "../destroyable.h"

class RenderPass : public Destroyable
{
private:
    std::optional<vk::CommandBuffer> currentCommandBuffer;

    vk::RenderPass renderPass;

    vk::ClearValue clearColor{};
    vk::RenderPassBeginInfo renderPassBeginInfo{};

public:
    RenderPass(vk::Device device, vk::RenderPass renderPass);
    void destroy() override;

    vk::RenderPass getRenderPass() { return renderPass; }

    void clear(float r, float g, float b, float a);
    void begin(vk::CommandBuffer commandBuffer);
    void end();
    void setFramebuffer(vk::Framebuffer buffer);
    void setRenderAreaExtent(vk::Extent2D extent);
};
