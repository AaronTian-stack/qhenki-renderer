#include "fxaa.h"

FXAA::FXAA(const char *shaderPath, PipelineBuilder &pipelineFactory, RenderPass *renderPass)
: PostProcess(shaderPath,pipelineFactory, renderPass)
{}

void FXAA::bindData(vk::CommandBuffer commandBuffer)
{
    pipeline->setPushConstant(commandBuffer, &fxaaData, sizeof(FXAAData), 0, vk::ShaderStageFlagBits::eFragment);
}
