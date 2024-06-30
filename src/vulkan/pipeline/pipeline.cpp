#include "pipeline.h"
#include <stdexcept>

Pipeline::Pipeline(vk::Device device, const vk::Pipeline &pipeline, const vk::PipelineLayout &pipelineLayout)
: Destroyable(device), pipeline(pipeline), pipelineLayout(pipelineLayout)
{}

void Pipeline::createGraphicsPipeline(const vk::GraphicsPipelineCreateInfo &pipelineInfo)
{
    // see vkCreateGraphicsPipelines
    auto result = device.createGraphicsPipeline(nullptr, pipelineInfo);
    pipeline = result.value;

    pipelineLayout = pipelineInfo.layout;
}

void Pipeline::destroy()
{
    device.destroyPipeline(pipeline);
    device.destroyPipelineLayout(pipelineLayout);
}

vk::PipelineLayout Pipeline::getPipelineLayout()
{
    return pipelineLayout;
}

vk::Pipeline Pipeline::getPipeline()
{
    return pipeline;
}

void Pipeline::setPushConstant(vk::CommandBuffer commandBuffer, void *value, size_t size, size_t offset, vk::ShaderStageFlags stages)
{
    commandBuffer.pushConstants(pipelineLayout, stages, offset, size, value);
}

