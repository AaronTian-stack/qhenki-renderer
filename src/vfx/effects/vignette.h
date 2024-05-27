#pragma once

#include "../postprocess.h"
#include "../../ui/menu.h"

struct VignetteData
{
    float intensity = 1.f;
    float vignetteX = 0.8f;
    float vignetteY = 0.25f;
    float saturation = 1.f;
    float saturationMul = 1.f;
};

class Vignette : public PostProcess, public Menu
{
private:
    VignetteData data;

public:
    Vignette(vk::Device device, const char* shaderPath, PipelineBuilder &pipelineFactory,
        DescriptorLayoutCache &layoutCache, RenderPass *renderPass);
    void bindData(vk::CommandBuffer commandBuffer) override;
    void renderMenu() override;
};

