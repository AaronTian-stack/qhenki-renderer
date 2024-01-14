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

VkPipeline Pipeline::getGraphicsPipeline()
{
    return graphicsPipeline;
}

void Pipeline::setPushConstant(vk::CommandBuffer commandBuffer, float constant)
{
    // TODO: change this more generic data than just a single float idk. should also account for offset (if stuff not in one giant struct)
    commandBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eAll, 0, sizeof(float), &constant);
}
