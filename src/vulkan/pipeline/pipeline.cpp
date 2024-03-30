#include "pipeline.h"
#include <stdexcept>

Pipeline::Pipeline(vk::Device device) : Destroyable(device) {}

vk::PipelineLayout Pipeline::createPipelineLayout(const vk::PipelineLayoutCreateInfo &pipelineLayoutInfo)
{
    auto layout = device.createPipelineLayout(pipelineLayoutInfo);
    pipelineLayout = layout;
    return pipelineLayout;
}

void Pipeline::createGraphicsPipeline(const vk::GraphicsPipelineCreateInfo &pipelineInfo)
{
    // see vkCreateGraphicsPipelines
    auto result = device.createGraphicsPipeline(nullptr, pipelineInfo);
    graphicsPipeline = result.value;
}

void Pipeline::destroy()
{
    device.destroyPipeline(graphicsPipeline);
    device.destroyPipelineLayout(pipelineLayout);
}

vk::PipelineLayout Pipeline::getPipelineLayout()
{
    return pipelineLayout;
}


vk::Pipeline Pipeline::getGraphicsPipeline()
{
    return graphicsPipeline;
}

void Pipeline::setPushConstant(vk::CommandBuffer commandBuffer, void *value, size_t size, size_t offset, vk::ShaderStageFlags stages)
{
    commandBuffer.pushConstants(pipelineLayout, stages, offset, size, value);
}
