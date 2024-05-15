#pragma once

#include "../postprocess.h"

struct FXAAData
{
    glm::vec2 resolution;
    // TODO: add more data here
};

class FXAA : public PostProcess
{
private:
    FXAAData fxaaData;

public:
    FXAA(const char* shaderPath, PipelineBuilder &pipelineFactory, RenderPass *renderPass);
    void bindData(vk::CommandBuffer commandBuffer) override;
};

