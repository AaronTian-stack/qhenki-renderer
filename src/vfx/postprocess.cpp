#include "postprocess.h"

PostProcess::PostProcess(const char *name, vk::Device device, const char *shaderPath, PipelineBuilder &pipelineFactory,
                         DescriptorLayoutCache &layoutCache, RenderPass *renderPass)
: Destroyable(device), name(name)
{
    shader = mkU<Shader>(device, "passthrough_vert.spv", shaderPath);
    pipelineFactory.reset();
    pipelineFactory.getColorBlending().attachmentCount = 1; // TODO: get rid of this eventually
    pipelineFactory.parseShader("passthrough_vert.spv", shaderPath, layoutCache, false);
    pipeline = pipelineFactory.buildPipeline(renderPass, 0, shader.get());
}

void PostProcess::destroy()
{
    shader->destroy();
    pipeline->destroy();
}

void PostProcess::bindData(vk::CommandBuffer commandBuffer)
{}
