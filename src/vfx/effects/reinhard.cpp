#include "reinhard.h"

Reinhard::Reinhard(vk::Device device, const char *shaderPath, PipelineBuilder &pipelineFactory,
           DescriptorLayoutCache &layoutCache, RenderPass *renderPass) : PostProcess("Reinhard", device, shaderPath,
                                                                                     pipelineFactory, layoutCache,
                                                                                     renderPass)
{}

void Reinhard::bindData(vk::CommandBuffer commandBuffer)
{

}
