#include "vignette.h"
#include "imgui/imgui.h"

void Vignette::bindData(vk::CommandBuffer commandBuffer)
{
    pipeline->setPushConstant(commandBuffer, &data, sizeof(VignetteData), 0, vk::ShaderStageFlagBits::eFragment);
}

Vignette::Vignette(vk::Device device, const char *shaderPath, PipelineBuilder &pipelineFactory,
           DescriptorLayoutCache &layoutCache, RenderPass *renderPass)
: PostProcess("Vignette", device, shaderPath,pipelineFactory, layoutCache,renderPass)
{}

void Vignette::renderMenu()
{
    ImGui::SliderFloat("Intensity", &data.intensity, 0.0f, 2.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    if (ImGui::SliderFloat("Outer Ring", &data.vignetteX, 0.0f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp))
    {
        if (data.vignetteX <= data.vignetteY)
            data.vignetteY = data.vignetteX - 0.001f;
    }
    if (ImGui::SliderFloat("Inner Falloff", &data.vignetteY, 0.0f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp))
    {
        if (data.vignetteY >= data.vignetteX)
            data.vignetteX = data.vignetteY + 0.001f;
    }
    ImGui::DragFloat("Saturation", &data.saturation, 0.01f, 10.f);
    ImGui::DragFloat("Saturation Mul", &data.saturationMul, 0.01f, 10.f);
}
