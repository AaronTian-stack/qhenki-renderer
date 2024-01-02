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
