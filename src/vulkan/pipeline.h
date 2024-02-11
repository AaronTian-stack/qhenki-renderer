#pragma once

#include <vulkan/vulkan.h>
#include "destroyable.h"

class Pipeline : public Destroyable
{
private:
    vk::PipelineLayout pipelineLayout;
    vk::Pipeline graphicsPipeline;
    void createGraphicsPipeline(const vk::GraphicsPipelineCreateInfo &pipelineInfo);
    vk::PipelineLayout createPipelineLayout(const vk::PipelineLayoutCreateInfo &pipelineLayoutInfo);

public:
    Pipeline(vk::Device device);

    void destroy() override;

    vk::PipelineLayout getPipelineLayout();
    vk::Pipeline getGraphicsPipeline();
    void setPushConstant(vk::CommandBuffer commandBuffer, float constant);

    friend class PipelineBuilder;
};
