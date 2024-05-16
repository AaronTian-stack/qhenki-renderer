#pragma once

#include "../postprocess.h"

class Vignette : public PostProcess
{
private:
    float intensity;

public:
    Vignette(vk::Device device, const char* shaderPath, PipelineBuilder &pipelineFactory,
        DescriptorLayoutCache &layoutCache, RenderPass *renderPass);
    void bindData(vk::CommandBuffer commandBuffer) override;

};

