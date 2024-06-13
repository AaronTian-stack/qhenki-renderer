#include "sharpen.h"
#include "imgui/imgui.h"

Sharpen::Sharpen(vk::Device device, const char *shaderPath, PipelineBuilder &pipelineFactory,
                 DescriptorLayoutCache &layoutCache, RenderPass *renderPass)
 : PostProcess("Sharpen", device, shaderPath,pipelineFactory, layoutCache, renderPass), intensity(0.8)
{}

void Sharpen::bindData(vk::CommandBuffer commandBuffer)
{
    pipeline->setPushConstant(commandBuffer, &intensity, sizeof(float), 0, vk::ShaderStageFlagBits::eFragment);
}
