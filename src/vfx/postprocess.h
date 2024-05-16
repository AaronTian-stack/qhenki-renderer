#pragma once

#include "../vulkan/pipeline/shader.h"
#include "../vulkan/pipeline/pipeline.h"
#include "../smartpointer.h"
#include "../vulkan/pipeline/pipelinefactory.h"

class PostProcess : public Destroyable
{
protected:
    uPtr<Shader> shader;
    uPtr<Pipeline> pipeline;

public:
    PostProcess(vk::Device device, const char* shaderPath, PipelineBuilder &pipelineFactory,
                DescriptorLayoutCache &layoutCache, RenderPass *renderPass);
    virtual void bindData(vk::CommandBuffer commandBuffer) = 0;
    void destroy() override;

    friend class PostProcessManager;
};