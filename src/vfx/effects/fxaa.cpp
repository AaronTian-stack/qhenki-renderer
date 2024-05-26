#include "fxaa.h"
#include "imgui/imgui.h"

void FXAA::bindData(vk::CommandBuffer commandBuffer)
{
    pipeline->setPushConstant(commandBuffer, &fxaaData, sizeof(FXAAData), 0, vk::ShaderStageFlagBits::eFragment);
}

FXAA::FXAA(vk::Device device, const char *shaderPath, PipelineBuilder &pipelineFactory,
           DescriptorLayoutCache &layoutCache, RenderPass *renderPass)
: PostProcess("FXAA", device, shaderPath,pipelineFactory, layoutCache, renderPass)
{}

void FXAA::renderMenu()
{
    ImGui::SliderFloat("Reduce Mul", &fxaaData.fxaaReduceMul, 0.0f, 256.0f);
    ImGui::SliderFloat("Reduce Min", &fxaaData.fxaaReduceMin, 0.0f, 16.0f);
    ImGui::SliderFloat("Span Max", &fxaaData.fxaaSpanMax, 0.0f, 16.0f);
}
