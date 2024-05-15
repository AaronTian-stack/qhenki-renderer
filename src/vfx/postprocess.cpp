#include "postprocess.h"

PostProcess::PostProcess(const char *shaderPath, PipelineBuilder &pipelineFactory, RenderPass *renderPass)
{
    shader = mkU<Shader>(device, "passthrough_vert.spv", shaderPath);
    pipelineFactory.reset();
    pipelineFactory.getColorBlending().attachmentCount = 1; // TODO: get rid of this eventually
    pipeline = pipelineFactory.buildPipeline(renderPass, 0, shader.get());
}

void PostProcess::destroy()
{
    shader->destroy();
    pipeline->destroy();
}
