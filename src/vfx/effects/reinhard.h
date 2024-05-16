#pragma once

#include "../postprocess.h"

class Reinhard : public PostProcess
{
public:
    Reinhard(vk::Device device, const char* shaderPath, PipelineBuilder &pipelineFactory,
        DescriptorLayoutCache &layoutCache, RenderPass *renderPass);
    void bindData(vk::CommandBuffer commandBuffer) override;
};

