#include "vulkanpipeline.h"
#include <stdexcept>

VulkanPipeline::VulkanPipeline() {}

VkPipelineLayout VulkanPipeline::createPipelineLayout(VkPipelineLayoutCreateInfo pipelineLayoutInfo)
{;
    if (vkCreatePipelineLayout(deviceForDispose, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
    return pipelineLayout;
}

void VulkanPipeline::createGraphicsPipeline(const VkGraphicsPipelineCreateInfo &pipelineInfo)
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

void VulkanPipeline::dispose()
{
    vkDestroyPipeline(deviceForDispose, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(deviceForDispose, pipelineLayout, nullptr);
}
