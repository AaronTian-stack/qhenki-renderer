#include "chromaticaberration.h"

void ChromaticAberration::bindData(vk::CommandBuffer commandBuffer)
{
    pipeline->setPushConstant(commandBuffer, &maxIntensity, sizeof(float), 0, vk::ShaderStageFlagBits::eFragment);
}

ChromaticAberration::ChromaticAberration(vk::Device device, const char *shaderPath, PipelineBuilder &pipelineFactory,
                     DescriptorLayoutCache &layoutCache, RenderPass *renderPass)
        : PostProcess("Chromatic Aberration", device, shaderPath,pipelineFactory, layoutCache,renderPass), maxIntensity(0.5f)
{}
