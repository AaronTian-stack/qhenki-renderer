#pragma once

#include "../postprocess.h"

struct FXAAData
{
    glm::vec2 resolution;
    float fxaaReduceMul = 128.f;
    float fxaaReduceMin = 8.f;
    float fxaaSpanMax = 8.f;
};

class FXAA : public PostProcess
{
private:
    FXAAData fxaaData;

public:
    FXAA(vk::Device device, const char* shaderPath, PipelineBuilder &pipelineFactory,
        DescriptorLayoutCache &layoutCache, RenderPass *renderPass);
    void bindData(vk::CommandBuffer commandBuffer) override;
    void setResolution(glm::vec2 resolution) { fxaaData.resolution = resolution; }
};

