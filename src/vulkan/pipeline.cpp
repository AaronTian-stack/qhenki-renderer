#include "pipeline.h"
#include <stdexcept>

Pipeline::Pipeline(VkDevice device) : Disposable(device) {}

VkPipelineLayout Pipeline::createPipelineLayout(VkPipelineLayoutCreateInfo pipelineLayoutInfo)
{;
    if (vkCreatePipelineLayout(deviceForDispose, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
    return pipelineLayout;
}

void Pipeline::createGraphicsPipeline(const VkGraphicsPipelineCreateInfo &pipelineInfo)
{
    if (vkCreateGraphicsPipelines(deviceForDispose,
                                  VK_NULL_HANDLE,
                                  1,
                                  &pipelineInfo,
                                  nullptr,
                                  &graphicsPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
}

void Pipeline::dispose()
{
    vkDestroyPipeline(deviceForDispose, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(deviceForDispose, pipelineLayout, nullptr);
}

VkPipeline Pipeline::getGraphicsPipeline()
{
    return graphicsPipeline;
}

void Pipeline::setPushConstant(VkCommandBuffer commandBuffer, float constant)
{
    // TODO: change this more generic data than just a single float idk. should also account for offset (if stuff not in one giant struct)
    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_ALL, 0, sizeof(float), &constant);
}
