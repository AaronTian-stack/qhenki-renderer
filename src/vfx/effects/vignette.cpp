#include "vignette.h"

void Vignette::bindData(vk::CommandBuffer commandBuffer)
{
    pipeline->setPushConstant(commandBuffer, &intensity, sizeof(float), 0, vk::ShaderStageFlagBits::eFragment);
}

Vignette::Vignette(vk::Device device, const char *shaderPath, PipelineBuilder &pipelineFactory,
           DescriptorLayoutCache &layoutCache, RenderPass *renderPass) : PostProcess("Vignette", device, shaderPath,
                                                                                     pipelineFactory, layoutCache,
                                                                                     renderPass)
{}
