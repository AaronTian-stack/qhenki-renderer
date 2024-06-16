#include "filmgrain.h"

void FilmGrain::bindData(vk::CommandBuffer commandBuffer)
{
    pipeline->setPushConstant(commandBuffer, &filmGrainData, sizeof(FilmGrainData), 0, vk::ShaderStageFlagBits::eFragment);
}

FilmGrain::FilmGrain(vk::Device device, const char *shaderPath, PipelineBuilder &pipelineFactory,
                   DescriptorLayoutCache &layoutCache, RenderPass *renderPass)
        : PostProcess("Film Grain", device, shaderPath,pipelineFactory, layoutCache,renderPass), filmGrainData({0.0f, 0.5f})
{}

void FilmGrain::updateTime(float time)
{
    filmGrainData.seed = time;
}
