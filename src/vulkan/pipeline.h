#pragma once

#include <vulkan/vulkan.h>
#include "../disposable.h"

class Pipeline : public Disposable
{
private:
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    void createGraphicsPipeline(const VkGraphicsPipelineCreateInfo &pipelineInfo);
    VkPipelineLayout createPipelineLayout(VkPipelineLayoutCreateInfo pipelineLayoutInfo);

public:
    Pipeline();

    void dispose() override;
    VkPipeline getGraphicsPipeline();

    friend class PipelineBuilder;
};
