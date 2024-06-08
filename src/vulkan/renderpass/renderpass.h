#pragma once

#include <optional>
#include "vulkan/vulkan.hpp"
#include "../destroyable.h"

class RenderPass : public Destroyable
{
private:
    std::optional<vk::CommandBuffer> currentCommandBuffer;

    vk::RenderPass renderPass;

    std::vector<vk::Format> formats;
    std::vector<vk::ClearValue> clearValues;
    vk::RenderPassBeginInfo renderPassBeginInfo{};

public:
    RenderPass(vk::Device device, vk::RenderPass renderPass, std::vector<vk::Format> formats);
    void destroy() override;

    vk::RenderPass getRenderPass() { return renderPass; }

    void clear(float r, float g, float b, float a);
    void doNotClear();
    void begin(vk::CommandBuffer commandBuffer);
    void nextSubpass();
    void end();
    void setFramebuffer(vk::Framebuffer buffer);
    void setRenderAreaExtent(vk::Extent2D extent);
};
