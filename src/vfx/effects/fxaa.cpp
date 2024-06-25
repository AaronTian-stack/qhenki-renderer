#include "fxaa.h"

void FXAA::bindData(vk::CommandBuffer commandBuffer)
{
    pipeline->setPushConstant(commandBuffer, &fxaaData, sizeof(FXAAData), 0, vk::ShaderStageFlagBits::eFragment);
}

FXAA::FXAA(vk::Device device, const char *shaderPath, PipelineBuilder &pipelineFactory,
           DescriptorLayoutCache &layoutCache, RenderPass *renderPass)
: PostProcess("FXAA", device, shaderPath,pipelineFactory, layoutCache, renderPass)
{}
