#pragma once

#include "../postprocess.h"
#include "../../ui/menu.h"

class Vignette : public PostProcess, public Menu
{
private:
    float intensity;

public:
    Vignette(vk::Device device, const char* shaderPath, PipelineBuilder &pipelineFactory,
        DescriptorLayoutCache &layoutCache, RenderPass *renderPass);
    void bindData(vk::CommandBuffer commandBuffer) override;
    void renderMenu() override;
};

