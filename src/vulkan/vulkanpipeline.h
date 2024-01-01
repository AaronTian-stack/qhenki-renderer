#pragma once

#include <vulkan/vulkan.h>
#include "../disposable.h"

class VulkanPipeline : public Disposable
{
private:
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    void createGraphicsPipeline(const VkGraphicsPipelineCreateInfo &pipelineInfo);
    VkPipelineLayout createPipelineLayout(VkPipelineLayoutCreateInfo pipelineLayoutInfo);

public:
    VulkanPipeline();

    void dispose() override;

    friend class VkPipelineBuilder;
};
