#pragma once

#include "../postprocess.h"
#include "../../ui/menu.h"

struct FXAAData
{
    float fxaaReduceMul = 128.f;
    float fxaaReduceMin = 8.f;
    float fxaaSpanMax = 8.f;
};

class FXAA : public PostProcess
{
public:
    FXAAData fxaaData;
    FXAA(vk::Device device, const char* shaderPath, PipelineBuilder &pipelineFactory,
        DescriptorLayoutCache &layoutCache, RenderPass *renderPass);
    void bindData(vk::CommandBuffer commandBuffer) override;
};

