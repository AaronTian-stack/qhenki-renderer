#pragma once

#include "../destroyable.h"
#include <vulkan/vulkan.h>

class Pipeline : public Destroyable
{
private:
    vk::Pipeline pipeline;
    vk::PipelineLayout pipelineLayout;
    void createGraphicsPipeline(const vk::GraphicsPipelineCreateInfo &pipelineInfo);

public:
    explicit Pipeline(vk::Device device, const vk::Pipeline &pipeline, const vk::PipelineLayout &pipelineLayout);

    void destroy() override;

    vk::PipelineLayout getPipelineLayout();
    vk::Pipeline getPipeline();
    void setPushConstant(vk::CommandBuffer commandBuffer, void *value, size_t size, size_t offset, vk::ShaderStageFlags stages);

    friend class PipelineBuilder;
};
